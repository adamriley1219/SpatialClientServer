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
#include "Shared/AIController.hpp"

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
	m_DevColsoleCamera.SetOrthographicProjection( Vec2( -100.0f, -50.0f ), Vec2( 100.0f,  50.0f ) );
	m_DevColsoleCamera.SetModelMatrix( Matrix44::IDENTITY );

	Zone* zone = Zone::AddZone( m_currentZone );
	zone->m_physics_system->SetGravity( Vec2::ZERO );
	
	LoadAbilities();
	LoadActors();

	m_clientController = new PlayerController();
	m_clientEntity = new ActorRenderable( "player" );
	m_clientEntity->Possess( m_clientController );

	ActorRenderable* ai_one = new ActorRenderable( "turret" );
	ai_one->Possess( new AIController() );
	ai_one->SetPosition( Vec2( 5.0f, 5.0f ) );

	ActorRenderable* ai_two = new ActorRenderable( "crawler" );
	ai_two->Possess( new AIController() );
	ai_two->SetPosition( Vec2( -5.0f, 5.0f ) );

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


#include "Game/SpatialOSClient.hpp"
//--------------------------------------------------------------------------
/**
* GameRender
*/
void Game::GameRender() const
{
	g_theRenderer->BindMaterial( g_theRenderer->CreateOrGetMaterialFromXML( "Data/Materials/default_unlit.mat" ) );

	Zone::GetZone(m_currentZone)->m_physics_system->DebugRender(g_theRenderer, Rgba::GREEN);

	auto entities = SpatialOSClient::GetEntityList();
	for( const entity_info_t& info : entities )
	{
		auto option = info.entity->Get<improbable::Position>();
		DebugRenderScreenPoint( 0.0f, Vec2( option->coords().x(), option->coords().y() ) );
	}

	worker::View* view = SpatialOSClient::GetView();
	if( view )
	{
		for (auto itrPair : view->Entities)
		{
			auto options = itrPair.second.Get<improbable::Position>();
			DebugRenderScreenPoint(0.0f, Vec2(options->coords().x(), options->coords().y()));
		}
	}

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
	Zone::GetZone(m_currentZone)->Update( deltaSeconds );

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
