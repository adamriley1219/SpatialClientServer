#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#include "Engine/Core/Debug/Log.hpp"

#include "Engine/Math/RNG.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Audio/AudioSystem.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Debug/DebugRenderSystem.hpp"
#include "Game/GameCommon.hpp"

//--------------------------------------------------------------------------
// Global Singletons
//--------------------------------------------------------------------------
RenderContext* g_theRenderer = nullptr;		// Created and owned by the App
InputSystem* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
App* g_theApp = nullptr;					// Created and owned by Main_Windows.cpp
bool g_isInDebug = false;
RNG* g_theRNG = nullptr;
PhysicsSystem* g_thePhysicsSystem = nullptr;
Game* g_theGame = nullptr;
WindowContext* g_theWindowContext = nullptr;


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
	g_thePhysicsSystem = new PhysicsSystem();
	g_theGame = new Game();

	LogSystemStartup("Data/Log/Log.txt");
	g_theEventSystem->Startup();
	g_theRenderer->Startup();
	g_theDebugRenderSystem->Startup();
	g_theConsole->Startup();
	g_thePhysicsSystem->Startup();
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
	g_thePhysicsSystem->Shutdown();
	g_theConsole->Shutdown();
	g_theDebugRenderSystem->Shutdown();
	g_theRenderer->Shutdown();
	g_theEventSystem->Shutdown();

	LogSystemShutdown();

	delete g_theGame;
	g_theGame = nullptr;
	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;
	delete g_theInputSystem;
	g_theInputSystem = nullptr;
	delete g_theConsole;
	g_theConsole = nullptr;
	delete g_theDebugRenderSystem;
	g_theDebugRenderSystem = nullptr;
	delete g_theRenderer;
	g_theRenderer = nullptr;
	delete g_theRNG;
	g_theRNG = nullptr;
}

//--------------------------------------------------------------------------
static float fpsGroup[50] = { 60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60 };
static uint fpsIdx = 0;
static float allFpsAccumilated = 60.0f * 50.0f;

static void addFPS( float deltaTime )
{
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
void App::RunFrame( float timeFrameBeganSec )
{
	double timeLastFrameSec = m_time;
	m_time = timeFrameBeganSec;

	float deltaTime = (float) ( timeFrameBeganSec - timeLastFrameSec );
	addFPS(deltaTime);

	deltaTime = Clamp( deltaTime, 0.0f, 0.1f );

	if( m_isPaused )
	{
		deltaTime = 0.0f;
	}
	else if( m_isSlowMo )
	{
		deltaTime *= 0.1f;
	}
	else if( m_isFastMo )
	{
		deltaTime *= 4.0f;
	}

	float fps = getAvgFPS();
	DebugRenderMessage( 0.0f, Rgba::YELLOW, Rgba::YELLOW, "%.2f", fps );

	BeginFrame();
	Update( deltaTime );
	Render();
	EndFrame();
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
	case 'p': // F1 press
		ToggleDebug();
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
* HandleKeyPressedTogglePause
*/
void App::TogglePause()
{
	m_isPaused = !m_isPaused;
}

//--------------------------------------------------------------------------
/**
* BeginFrame
*/
void App::BeginFrame()
{
	g_theDebugRenderSystem->BeginFrame();
	g_theEventSystem->		BeginFrame();
	g_theRenderer->			BeginFrame();
	g_theConsole->			BeginFrame();
	g_theInputSystem->		BeginFrame();
	g_theAudioSystem->		BeginFrame();

}


//--------------------------------------------------------------------------
/**
* Update
*/
void App::Update( float deltaSeconds )
{
	g_theConsole->	Update();
	g_theGame->		UpdateGame( deltaSeconds );
	g_theDebugRenderSystem->Update();
}

//--------------------------------------------------------------------------
/**
* RenderDebugRenderDebugLeftJoystick
*/
void App::RenderDebugLeftJoystick() const
{
	float inRangex = 2.0f;
	float inRangey = 2.0f;
	float outerRadius = 8.0f;
	float posRadius = 0.5f;
	const XboxController& curController = g_theInputSystem->GetControllerByID(0);
	if( !curController.IsConnected() )
		return;
	const AnalogJoystick& curLJoystick = curController.GetLeftJoystick();
	const Vec2& upRightRef = g_theGame->m_DevColsoleCamera.GetOrthoTopRight();

	Vec3 center
	(	
		upRightRef.x - inRangex - outerRadius
		,	upRightRef.y - inRangey - outerRadius
		,	0.0f
	);

	Vertex_PCU centerVert( center, Rgba( 0.4f, 0.4f, 0.4f, 0.5f ), Vec2( 0.0f, 0.0f ) );
	DrawDisc( centerVert , outerRadius );



	centerVert.color.r = 0.1f;
	centerVert.color.g = 0.1f;
	centerVert.color.b = 0.1f;
	DrawDisc( centerVert , outerRadius * curLJoystick.GetOuterDeadZoneFraction() );
	centerVert.color.r = 0.3f;
	centerVert.color.g = 0.3f;
	centerVert.color.b = 0.3f;
	DrawDisc( centerVert , outerRadius * curLJoystick.GetInnerDeadZoneFraction() );

	Vec3 rawCenter
	(
		center.x + curLJoystick.GetRawPosition().x * outerRadius
		,	center.y + curLJoystick.GetRawPosition().y * outerRadius
		,	0.0f	
	);
	Vertex_PCU rawInput( rawCenter, Rgba( 1.0f, 0.0f, 0.0f, 1.0f ), Vec2( 0.0f, 0.0f ) );
	DrawDisc( rawInput , posRadius );

	Vec3 fixedCenter
	(
		center.x + curLJoystick.GetPosition().x * outerRadius
		,	center.y + curLJoystick.GetPosition().y * outerRadius
		,	0.0f	
	);
	Vertex_PCU fixedInput( fixedCenter, Rgba( 0.0f, 0.7f, 0.7f, 1.0f ), Vec2( 0.0f, 0.0f ) );
	DrawDisc( fixedInput , posRadius );
}

//--------------------------------------------------------------------------
/**
* Render
*/
void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba::BLACK );
	g_theGame->GameRender();

	if (g_theConsole->IsOpen())
	{
		g_theConsole->Render(g_theRenderer, g_theGame->m_DevColsoleCamera, m_consoleTextHeight);
	}
	else
	{
		g_theDebugRenderSystem->RenderToScreen();
	}
}

//--------------------------------------------------------------------------
/**
* EndFrame
*/
void App::EndFrame()
{
	g_theDebugRenderSystem->EndFrame();
	g_theConsole->			EndFrame();
	g_theAudioSystem->		EndFrame();
	g_theRenderer->			EndFrame();
	g_theEventSystem->		EndFrame();
	g_theInputSystem->		EndFrame();
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

