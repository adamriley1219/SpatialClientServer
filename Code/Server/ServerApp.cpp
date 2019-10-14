#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#include "Engine/Core/Debug/Log.hpp"
#include "Engine/Core/Debug/Profiler.hpp"
#include "Engine/Core/Time/Clock.hpp"

#include "Engine/Math/RNG.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

#include "Server/ServerApp.hpp"

//--------------------------------------------------------------------------
// Global Singletons
//--------------------------------------------------------------------------
ServerApp* g_theServerApp = nullptr;					// Created and owned by Main_Windows.cpp
PhysicsSystem* g_thePhysicsSystem = nullptr;


//--------------------------------------------------------------------------
/**
* Startup
*/
void ServerApp::Startup()
{
	g_thePhysicsSystem = new PhysicsSystem();

	LogSystemStartup("Data/Log/Log.txt");
	ProfilerSystemInit();

	ClockSystemStartup();
	m_gameClock = new Clock(&Clock::Master);

	g_thePhysicsSystem->Startup();


	RegisterEvents();
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void ServerApp::Shutdown()
{
	g_thePhysicsSystem->Shutdown();

	ProfilerSystemDeinit();
	LogSystemShutdown();


	SAFE_DELETE(m_gameClock);
	SAFE_DELETE(g_thePhysicsSystem);
}

//--------------------------------------------------------------------------
/**
* RunFrame
*/
void ServerApp::RunFrame()
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


	BeginFrame();
	Update( (float)m_gameClock->GetFrameTime() );
	EndFrame();

	ProfilerEndFrame();
}

//--------------------------------------------------------------------------
/**
* HandleQuitRequest
*/
void ServerApp::HandleQuitRequested()
{
	m_isQuitting = true;
}

//--------------------------------------------------------------------------
/**
* QuitEvent
*/
bool ServerApp::QuitEvent( EventArgs& args )
{
	UNUSED( args );
	g_theServerApp->HandleQuitRequested();
	return true;
}


//--------------------------------------------------------------------------
/**
* BeginFrame
*/
void ServerApp::BeginFrame()
{
	PROFILE_FUNCTION();
	ClockSystemBeginFrame();
	g_thePhysicsSystem->	BeginFrame();

}


//--------------------------------------------------------------------------
/**
* Update
*/
void ServerApp::Update( float deltaSeconds )
{
	PROFILE_FUNCTION();
	g_thePhysicsSystem->Update( deltaSeconds );
}


//--------------------------------------------------------------------------
/**
* EndFrame
*/
void ServerApp::EndFrame()
{
	PROFILE_FUNCTION();
	g_thePhysicsSystem->	EndFrame();
}

//--------------------------------------------------------------------------
/**
* RegisterEvents
*/
void ServerApp::RegisterEvents()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "quit", QuitEvent );
}

