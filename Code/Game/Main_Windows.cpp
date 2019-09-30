

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <shellapi.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Engine/Core/Time/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/Strings/NamedStrings.hpp"
#include "Engine/Core/XML/XMLUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

//--------------------------------------------------------------------------
//SpatialOS
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <iostream>

// Use this to make a worker::ComponentRegistry. This worker doesn't use any components yet
// For example use worker::Components<improbable::Position, improbable::Metadata> to track these common components
using EmptyRegistry = worker::Components<>;

// Constants and parameters
const int ErrorExitStatus = 1;
const std::string kLoggerName = "client";
const std::uint32_t kGetOpListTimeoutInMilliseconds = 100;

// Connection helpers
worker::Connection ConnectWithLocator(const std::string hostname,
	const std::string project_name,
	const std::string deployment_id,
	const std::string login_token,
	const worker::ConnectionParameters& connection_parameters) {
	worker::LocatorParameters locator_parameters;
	locator_parameters.ProjectName = project_name;
	locator_parameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
	locator_parameters.LoginToken.Token = login_token;

	worker::Locator locator{ hostname, locator_parameters };

	auto queue_status_callback = [&](const worker::QueueStatus& queue_status) {
		if (!queue_status.Error.empty()) {
			std::cerr << "Error while queueing: " << *queue_status.Error << std::endl;
			return false;
		}
		std::cout << "Worker of type '" << connection_parameters.WorkerType
			<< "' connecting through locator: queueing." << std::endl;
		return true;
	};

	auto future = locator.ConnectAsync(EmptyRegistry{}, deployment_id, connection_parameters, queue_status_callback);
	return future.Get();
}


worker::Connection ConnectWithReceptionist(const std::string hostname,
	const std::uint16_t port,
	const std::string& worker_id,
	const worker::ConnectionParameters& connection_parameters) {
	auto future = worker::Connection::ConnectAsync(EmptyRegistry{}, hostname, port, worker_id, connection_parameters);
	return future.Get();
}

std::string get_random_characters(size_t count) {
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

constexpr float CLIENT_ASPECT = 2.0f; 
const char* APP_NAME = "Protogame3D";					

//-----------------------------------------------------------------------------------------------
static bool AppWindowProc( void* windowHandle, uint32_t wmMessageCode, uintptr_t wParam, uintptr_t lParam )
{
	UNREFERENCED_PARAMETER(windowHandle); 
	UNREFERENCED_PARAMETER(wParam); 
	UNREFERENCED_PARAMETER(lParam); 

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

	g_theApp->RunFrame( (float) GetCurrentTimeSeconds() );	

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

	Startup();

	LPWSTR* argv;
	int argc;

	argv = CommandLineToArgvW( GetCommandLineW(), &argc);

	auto now = std::chrono::high_resolution_clock::now();
	std::srand((uint)std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count());

	auto print_usage = [&]() {
		std::cout << "Usage: External receptionist <hostname> <port> <worker_id>" << std::endl;
		std::cout << "       External locator <hostname> <project_name> <deployment_id> <login_token>";
		std::cout << std::endl;
		std::cout << "Connects to SpatialOS" << std::endl;
		std::cout << "    <hostname>       - hostname of the receptionist or locator to connect to.";
		std::cout << std::endl;
		std::cout << "    <port>           - port to use if connecting through the receptionist.";
		std::cout << std::endl;
		std::cout << "    <worker_id>      - name of the worker assigned by SpatialOS." << std::endl;
		std::cout << "    <project_name>   - name of the project to run." << std::endl;
		std::cout << "    <deployment_id>  - name of the cloud deployment to run." << std::endl;
		std::cout << "    <login_token>   - token to use when connecting through the locator.";
		std::cout << std::endl;
	};

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "External";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = true;

	std::vector<std::string> arguments;

	// if no arguments are supplied, use the defaults for a local deployment
	if (argc == 1) {
		arguments = { "receptionist", "localhost", "7777", parameters.WorkerType + "_" + get_random_characters(4) };
	}
	else {
		for( uint i = 1; i < argc; ++i )
		{
			arguments.push_back( std::string( (char*)argv[i] ) );
		}
	}

	const std::string connection_type = arguments[0];
	if (connection_type != "receptionist" && connection_type != "locator") {
		print_usage();
		return ErrorExitStatus;
	}

	const bool use_locator = connection_type == "locator";

	if ((use_locator && arguments.size() != 5) || (!use_locator && arguments.size() != 4)) {
		print_usage();
		return ErrorExitStatus;
	}

	// Connect with locator or receptionist
	worker::Connection connection = use_locator
		? ConnectWithLocator(arguments[1], arguments[2], arguments[3], arguments[4], parameters)
		: ConnectWithReceptionist(arguments[1], atoi(arguments[2].c_str()), arguments[3], parameters);

	connection.SendLogMessage(worker::LogLevel::kInfo, kLoggerName, "Connected successfully");

	// Register callbacks and run the worker main loop.
	worker::Dispatcher dispatcher{ EmptyRegistry{} };
	bool is_connected = connection.IsConnected();

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		std::cerr << "[disconnect] " << op.Reason << std::endl;
		is_connected = false;
		});

	// Print messages received from SpatialOS
	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		if (op.Level == worker::LogLevel::kFatal) {
			std::cerr << "Fatal error: " << op.Message << std::endl;
			std::terminate();
		}
		std::cout << "Connection: " << op.Message << std::endl;
		});


	//--------------------------------------------------------------------------

	// Program main loop; keep running frames until it's time to quit
	while( !g_theApp->IsQuitting() && is_connected) 
	{
		dispatcher.Process(connection.GetOpList(kGetOpListTimeoutInMilliseconds));
		RunFrame();
		Sleep(0);
	}

	Shutdown();
	return ErrorExitStatus;
}

#endif
