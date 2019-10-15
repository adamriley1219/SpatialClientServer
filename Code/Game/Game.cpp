#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec2.hpp"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Shaders/Shader.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Debug/DebugRenderSystem.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/ActorRenderable.hpp"
#include "Game/PlayerController.hpp"

#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/Zone.hpp"

#include <vector>

#include <math.h>
//--------------------------------------------------------------------------
/**
* Game
*/
Game::Game()
{
	ConstructGame();
}

//--------------------------------------------------------------------------
/**
* ~Game
*/
Game::~Game()
{
	DeconstructGame();
}

//--------------------------------------------------------------------------
/**
* GameStartup
*/
void Game::Startup()
{
	m_shader = g_theRenderer->CreateOrGetShaderFromXML( "Data/Shaders/shader.xml" );
	g_theRenderer->m_shader = m_shader;

	m_DevColsoleCamera.SetOrthographicProjection( Vec2( -100.0f, -50.0f ), Vec2( 100.0f,  50.0f ) );
	m_DevColsoleCamera.SetModelMatrix( Matrix44::IDENTITY );

	Zone* zone = Zone::AddZone( m_currentZone );
	zone->m_physics_system->SetGravity( Vec2::ZERO );
	
	LoadAbilities();
	LoadActors();

	m_clientEntity = new ActorRenderable( "player" );
	m_clientController = new PlayerController( m_clientEntity );

	m_curentCamera.SetModelMatrix( Matrix44::IDENTITY );
	m_curentCamera.SetOrthographicProjection( Vec2( -25.0f, -12.5f ), Vec2( 25.0f, 12.5f ) );

	
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void Game::Shutdown()
{
	Zone::ClearAllZones();
}

//static int g_index = 0;

//--------------------------------------------------------------------------
/**
* HandleKeyPressed
*/
bool Game::HandleKeyPressed( unsigned char keyCode )
{
	UNUSED(keyCode);
	return false;
}

//--------------------------------------------------------------------------
/**
* HandleKeyReleased
*/
bool Game::HandleKeyReleased( unsigned char keyCode )
{
	UNUSED(keyCode);
	return false;
}



//--------------------------------------------------------------------------
/**
* GameRender
*/
void Game::GameRender() const
{
	g_theRenderer->BindMaterial( g_theRenderer->CreateOrGetMaterialFromXML( "Data/Materials/default_unlit.mat" ) );

	Zone::GetZone(m_currentZone)->m_physics_system->DebugRender(g_theRenderer, Rgba::GREEN);

	std::vector<Vertex_PCU> verts;
	AddVertsForRing2D(verts, Vec2::ZERO, 5.0f, 0.5f, Rgba::CYAN);

	g_theRenderer->DrawVertexArray(verts);

	g_theDebugRenderSystem->RenderToCamera( &g_theGame->m_curentCamera );
}

//--------------------------------------------------------------------------
/**
* UpdateGame
*/
void Game::UpdateGame( float deltaSeconds )
{
	m_clientController->Update( deltaSeconds );

	Zone::GetZone(m_currentZone)->Update(deltaSeconds);

	UpdateCamera( deltaSeconds );
}


//--------------------------------------------------------------------------
/**
* BeginCamera
*/
void Game::BeginCamera()
{
	//Begin Rendering
	m_curentCamera.SetColorTargetView(g_theRenderer->GetColorTargetView());
	m_curentCamera.SetDepthTargetView(g_theRenderer->GetDepthTargetView());
	g_theRenderer->BeginCamera(&m_curentCamera);
}

//--------------------------------------------------------------------------
/**
* EndCamera
*/
void Game::EndCamera()
{
	// End Rendering
	g_theRenderer->EndCamera();
}

//--------------------------------------------------------------------------
/**
* GetCamera
*/
Camera* Game::GetCamera()
{
	return &m_curentCamera;
}

//--------------------------------------------------------------------------
/**
* RenderDevConsole
*/
void Game::RenderDevConsole() const
{
	g_theConsole->Render( g_theRenderer, m_DevColsoleCamera, 10 );
}

//--------------------------------------------------------------------------
/**
* UpdateCamera
*/
void Game::UpdateCamera( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
}

//--------------------------------------------------------------------------
/**
* ResetGame
*/
void Game::ResetGame()
{
	DeconstructGame();
	ConstructGame();
}

//--------------------------------------------------------------------------
/**
* ConstructGame
*/
void Game::ConstructGame()
{

}

//--------------------------------------------------------------------------
/**
* DeconstructGame
*/
void Game::DeconstructGame()
{

}
