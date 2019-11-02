#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#include "Engine/Core/Debug/Log.hpp"
#include "Engine/Core/Debug/Profiler.hpp"
#include "Engine/Core/Time/Clock.hpp"

#include "Engine/Math/RNG.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Audio/AudioSystem.hpp"

#include "Engine/ImGUI/ImGUI_Interfaces/ProfilerDisplay.hpp"
#include "Engine/ImGUI/ImGUISystem.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Debug/DebugRenderSystem.hpp"
#include "Game/GameCommon.hpp"

#include "Shared/Zone.hpp"

//--------------------------------------------------------------------------
// Global Singletons
//--------------------------------------------------------------------------
RenderContext* g_theRenderer = nullptr;		// Created and owned by the App
InputSystem* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
App* g_theApp = nullptr;					// Created and owned by Main_Windows.cpp
bool g_isInDebug = false;
RNG* g_theRNG = nullptr;
Game* g_theGame = nullptr;
WindowContext* g_theWindowContext = nullptr;
ImGUISystem* g_theImGUISystem = nullptr;


//--------------------------------------------------------------------------
/**
* Startup
*/
void App::Startup()
{
	g_theRNG = new RNG();
	g_theEventSystem = new EventSystem();
	g_theConsole = new DevConsole( "SquirrelFixedFont" );
	g_theRenderer = new RenderContext( g_theWindowContext );
	g_theDebugRenderSystem = new DebugRenderSystem(g_theRenderer, 50.0f, 100.0f, "SquirrelFixedFont");
	g_theInputSystem = new InputSystem();
	g_theAudioSystem = new AudioSystem();
	g_theImGUISystem = new ImGUISystem(g_theRenderer);

	g_theGame = new Game();


	LogSystemStartup("Data/Log/Log.txt");
	ProfilerSystemInit();

	ClockSystemStartup();
	m_gameClock = new Clock(&Clock::Master);

	g_theEventSystem->Startup();
	g_theRenderer->Startup();
	g_theDebugRenderSystem->Startup();
	g_theConsole->Startup();
	g_theImGUISystem->Startup();

	g_theGame->Startup();

	EventArgs args;
	g_theDebugRenderSystem->Command_Open(args);

	RegisterEvents();
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void App::Shutdown()
{
	g_theGame->Shutdown();
	g_theImGUISystem->Shutdown();
	g_theConsole->Shutdown();
	g_theDebugRenderSystem->Shutdown();
	g_theRenderer->Shutdown();
	g_theEventSystem->Shutdown();

	ProfilerSystemDeinit();
	LogSystemShutdown();

	SAFE_DELETE(g_theGame);

	SAFE_DELETE(m_gameClock);
	SAFE_DELETE(g_theImGUISystem);
	SAFE_DELETE(g_theAudioSystem);
	SAFE_DELETE(g_theInputSystem);
	SAFE_DELETE(g_theConsole);
	SAFE_DELETE(g_theDebugRenderSystem);
	SAFE_DELETE(g_theRenderer);
	SAFE_DELETE(g_theEventSystem);
	SAFE_DELETE(g_theRNG);
}

//--------------------------------------------------------------------------
static float fpsGroup[50] = { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 };
static uint fpsIdx = 0;
static float allFpsAccumilated = 60.0f * 50.0f;

static void addFPS( float deltaTime )
{
	if( deltaTime <= 0.0f )
	{
		return;
	}
	allFpsAccumilated -= fpsGroup[fpsIdx];
	fpsGroup[fpsIdx] = 1.0f / deltaTime;
	allFpsAccumilated += fpsGroup[fpsIdx];
	fpsIdx++;
	if( fpsIdx >= 50 )
	{
		fpsIdx = 0;
	}
}

static float getAvgFPS()
{
	return allFpsAccumilated / 50.0f;
}
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
* RunFrame
*/
void App::RunFrame()
{
	ProfilerBeginFrame();
	if (m_isSlowMo)
	{
		m_gameClock->Dilate(0.1f);
	}
	else if (m_isFastMo)
	{
		m_gameClock->Dilate(4.0f);
	}
	else
	{
		m_gameClock->Dilate(1.0f);
	}

	addFPS( (float)m_gameClock->GetFrameTime() );
	float fps = getAvgFPS();
	DebugRenderMessage( 0.0f, Rgba::YELLOW, Rgba::YELLOW, "%.2f", fps );

	BeginFrame();
	Update( (float)m_gameClock->GetFrameTime() );
	Render();
	EndFrame();

	ProfilerEndFrame();
}

//--------------------------------------------------------------------------
/**
* HandleKeyPressed
*/
bool App::HandleKeyPressed( unsigned char keyCode )
{
	if( g_theConsole->HandleKeyPress( keyCode ) )
	{
		return true;
	}
	switch( keyCode )
	{
	
	case 192: // '~' press

		break;
	case 'P':
		if(m_isPaused)
			m_isPaused = false;
		else
			m_isPaused = true;
		return true;
		break;
	case 'T':
		m_isSlowMo = true;
		return true;
		break;
	case 'Y':
		m_isFastMo = true;
		return true;
		break;
	case 'w': // F8 press
		delete g_theGame;
		g_theGame = new Game();
		return true;
		break;
	case 112: // F1
		ProfilerToggleDisply();
		ToggleDebug();
		return true;
		break;
	case 113: // F2
		ProfilerTogglePaused();
		return true;
		break;
	case 114: // F2
		ProfilerToggleTreeMode();
		return true;
		break;
	default:
		return g_theGame->HandleKeyPressed( keyCode );
		break;
	}
	return false;
}

//--------------------------------------------------------------------------
/**
* HandleCharPressed
*/
bool App::HandleCharPressed( unsigned char keyCode )
{
	if( g_theConsole->HandleCharPress( keyCode ) )
	{
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
/**
* HandleKeyReleased
*/
bool App::HandleKeyReleased( unsigned char keyCode )
{
	if( g_theConsole->HandleKeyReleased( keyCode ) )
	{
		return true;
	}
	switch( keyCode )
	{
	case 'T':
		m_isSlowMo = false;
		break;
	case 'Y':
		m_isFastMo = false;
		break;
	case 'H':
		g_theEventSystem->FireEvent( "help" );
		break;
	default:
		return g_theGame->HandleKeyReleased( keyCode );
		break;
	}
	return true;
}

//--------------------------------------------------------------------------
/**
* HandleQuitRequested
*/
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return true;
}

//--------------------------------------------------------------------------
/**
* QuitEvent
*/
bool App::QuitEvent( EventArgs& args )
{
	UNUSED( args );
	g_theApp->HandleQuitRequested();
	return true;
}


//--------------------------------------------------------------------------
/**
* IsPaused
*/
bool App::IsPaused() const
{
	return m_gameClock->IsPaused();
}

//--------------------------------------------------------------------------
/**
* Unpause
*/
void App::Unpause()
{
	m_gameClock->Resume();
}

//--------------------------------------------------------------------------
/**
* Pause
*/
void App::Pause()
{
	m_gameClock->Pause();
}

//--------------------------------------------------------------------------
/**
* TogglePause
*/
void App::TogglePause()
{
	if( IsPaused() )
	{
		Unpause();
	}
	else
	{
		Pause();
	}
}

//--------------------------------------------------------------------------
/**
* GetGlobleTime
*/
float App::GetGlobleTime() const
{
	return (float) m_gameClock->GetTotalTime();
}

//--------------------------------------------------------------------------
/**
* BeginFrame
*/
void App::BeginFrame()
{
	PROFILE_FUNCTION();
	ClockSystemBeginFrame();
	g_theImGUISystem->		BeginFrame();
	g_theEventSystem->		BeginFrame();
	g_theRenderer->			BeginFrame();
	g_theConsole->			BeginFrame();
	g_theInputSystem->		BeginFrame();
	g_theAudioSystem->		BeginFrame();
	g_theDebugRenderSystem->BeginFrame();

	Zone::BeginFrame();
}


//--------------------------------------------------------------------------
/**
* Update
*/
void App::Update( float deltaSeconds )
{
	PROFILE_FUNCTION();
	g_theConsole->	Update();
	g_theGame->		UpdateGame( deltaSeconds );
	g_theDebugRenderSystem->Update();
}

//--------------------------------------------------------------------------
/**
* Render
*/
void App::Render() const
{
	PROFILE_FUNCTION();

	g_theGame->BeginCamera();
	g_theRenderer->ClearScreen( Rgba::BLACK );

	g_theGame->GameRender();

	ProfilerDisplayRender();
	g_theImGUISystem->Render();

	if (g_theConsole->IsOpen())
	{
		g_theConsole->Render(g_theRenderer, g_theGame->m_DevColsoleCamera, m_consoleTextHeight);
	}
	else
	{
		g_theDebugRenderSystem->RenderToScreen();
	}
	g_theGame->EndCamera();
}

//--------------------------------------------------------------------------
/**
* EndFrame
*/
void App::EndFrame()
{
	PROFILE_FUNCTION();

	Zone::EndFrame();
	g_theDebugRenderSystem->EndFrame();
	g_theConsole->			EndFrame();
	g_theAudioSystem->		EndFrame();
	g_theRenderer->			EndFrame();
	g_theEventSystem->		EndFrame();
	g_theInputSystem->		EndFrame();
	g_theImGUISystem->		EndFrame();
}

//--------------------------------------------------------------------------
/**
* ToggleDebug
*/
void App::ToggleDebug()
{
	if ( g_isInDebug )
	{	
		g_isInDebug = false;
	}
	else
	{
		g_isInDebug = true;
	}
}

//--------------------------------------------------------------------------
/**
* RegisterEvents
*/
void App::RegisterEvents()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "quit", QuitEvent );
}

