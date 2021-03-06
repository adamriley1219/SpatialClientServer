

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <shellapi.h>
#include <math.h>

#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/Strings/NamedStrings.hpp"
#include "Engine/Core/XML/XMLUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Engine/Memory/Debug/Log.hpp"

#include "Engine/ImGUI/ImGUISystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Game/SpatialOSClient.hpp"

#include <algorithm>

constexpr float CLIENT_ASPECT = 2.0f; 
const char* APP_NAME = "Client";					



static std::string get_random_characters(size_t count) {
	std::srand((uint)GetCurrentTimeHPC());
	const auto randchar = []() -> char {
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const auto max_index = sizeof(charset) - 1;
		return charset[std::rand() % max_index];
	};
	std::string str(count, 0);
	std::generate_n(str.begin(), count, randchar);
	return str;
}

//-----------------------------------------------------------------------------------------------
static bool AppWindowProc( void* windowHandle, uint32_t wmMessageCode, uintptr_t wParam, uintptr_t lParam )
{
	if (ImGUI_ProcessInputWindows(windowHandle, wmMessageCode, wParam, lParam))
	{
		// ImGUI Consumed it
		return true;
	}

	switch( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
	case WM_CLOSE:		
	{
		//g_isQuitting = true;
		g_theApp->HandleQuitRequested();
		return true; // "Consumes" this message (tells Windows "okay, we handled it")
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		unsigned char asKey = (unsigned char) wParam;
		g_theApp->HandleKeyPressed(asKey);

		if( asKey == VK_ESCAPE )
		{
			if( !g_theConsole->HandleESCPress() )
			{
				g_theEventSystem->FireEvent( "quit" );
			}
			//g_theApp->HandleQuitRequested();
			return true; 
		}

		return true;
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		unsigned char asKey = (unsigned char) wParam;
		if(g_theApp->HandleKeyReleased(asKey))
		{
			return true; 
		}

		break;
	}
	case WM_CHAR:
	{
		unsigned char asKey = (unsigned char) wParam;
		if(g_theApp->HandleCharPressed(asKey))
		{
			return true;
		}

		break;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return false; 
}

//-----------------------------------------------------------------------------------------------
void CreateWindowAndRenderContext( float clientAspect )
{
	g_theWindowContext = new WindowContext();
	g_theWindowContext->Create( APP_NAME, clientAspect, .90f, AppWindowProc ); 
}
//-----------------------------------------------------------------------------------------------
void RunMessagePump()
{
	g_theWindowContext->BeginFrame(); 
}

//-----------------------------------------------------------------------------------------------
void RunFrame()
{
	RunMessagePump();

	g_theApp->RunFrame();	

}

//-----------------------------------------------------------------------------------------------
void Startup()
{
	tinyxml2::XMLDocument config;
	config.LoadFile("Data/GameConfig.xml");
	XmlElement* root = config.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*root);
	CreateWindowAndRenderContext( CLIENT_ASPECT );
	g_theApp = new App();
	g_theApp->Startup();
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	g_theApp->Shutdown();

	// Destroy the global App instance
	delete g_theApp;		
	g_theApp = nullptr;

	delete g_theWindowContext; 
	g_theWindowContext = nullptr; 

}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( applicationInstanceHandle ); 
	UNUSED( commandLineString );


	char** argv = __argv;
	int argc = __argc;
	
	//argv = CommandLineToArgvW( GetCommandLineW(), &argc);

	std::vector<std::string> arguments;

	bool use_cloud = true;

	// if no arguments are supplied, use the defaults for a local deployment
	if( use_cloud )
	{
					//	type,	hostname,  port,      project name,        deployment name, login token 
		arguments = { "locator", "locator.improbable.io", "443", "beta_cat_grey_781", "origin_build", "ZWQxMjRlMmUtMTJiZi00ZmRkLTk1MzAtOTM2NDRlMzBjMTBiOjoxZDFiNzYyZi1hMDc2LTQxNTgtODYzMC0zOTNkMThhZjI3MTk=" };
	} 
	else if (argc == 1) 
	{
		arguments = { "receptionist", "localhost", "7790", "External_" + get_random_characters(4) };
	}
	else 
	{
		for( uint i = 1; i < (uint)argc; ++i )
		{
			arguments.push_back( std::string( (char*)argv[i] ) );
		}
	}

	LogSystemStartup("Data/Log/Log.txt");
	//--------------------------------------------------------------------------
	// Program main loop; keep running frames until it's time to quit

	SpatialOSClient::Startup( arguments );
	Startup();


	// Only needed if spatialOS thread needs to begin, if not using a thread you can bypass the sleep
//	Sleep( 1000 );

	while( !g_theApp->IsQuitting() && SpatialOSClient::IsRunning() ) 
	{
		RunFrame();
		Sleep(0);
	}

	Shutdown();
	SpatialOSClient::Shutdown();

	LogSystemShutdown();
	return 0;
}

#endif
