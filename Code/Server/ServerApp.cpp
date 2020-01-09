#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#include "Engine/Core/Time/Clock.hpp"

#include "Engine/Math/RNG.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"

#include "Engine/Math/RNG.hpp"

#include "Server/ServerApp.hpp"
#include "Server/SpatialOSServer.hpp"
#include "Server/ServerCommon.hpp"
#include "Server/WorldSim.hpp"

#include "Shared/Zone.hpp"



//--------------------------------------------------------------------------
// Global Singletons
//--------------------------------------------------------------------------
ServerApp* g_theServerApp = nullptr;					// Created and owned by Main_Windows.cpp
WorldSim* g_theSim = nullptr;
RNG* g_theRNG = nullptr;

//--------------------------------------------------------------------------
/**
* Startup
*/
void ServerApp::Startup()
{
	g_theRNG = new RNG();

	g_theEventSystem = new EventSystem();
	g_theEventSystem->Startup();

	std::cout << "Server clock startup" << std::endl;
	ClockSystemStartup();
	m_gameClock = new Clock(&Clock::Master);

	std::cout << "Server Zone startup" << std::endl;
	Zone::Startup();

	std::cout << "World sim startup" << std::endl;
	g_theSim = new WorldSim();
	g_theSim->Startup();

	std::cout << "Server Events startup" << std::endl;
	RegisterEvents();
	std::cout << "Server startup complete" << std::endl;

}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void ServerApp::Shutdown()
{
	g_theSim->Shutdown();

	Zone::Shutdown();

	SAFE_DELETE(m_gameClock);
	SAFE_DELETE(g_theSim);
	SAFE_DELETE(g_theRNG);
}

//--------------------------------------------------------------------------
/**
* RunFrame
*/
void ServerApp::RunFrame()
{
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
	ClockSystemBeginFrame();

	SpatialOSServer::Process();
	Zone::BeginFrame();
}


//--------------------------------------------------------------------------
/**
* Update
*/
void ServerApp::Update( float deltaSeconds )
{
	Zone::UpdateZones( deltaSeconds );
}


//--------------------------------------------------------------------------
/**
* EndFrame
*/
void ServerApp::EndFrame()
{
	Zone::EndFrame();
}

//--------------------------------------------------------------------------
/**
* RegisterEvents
*/
void ServerApp::RegisterEvents()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "quit", QuitEvent );
}

