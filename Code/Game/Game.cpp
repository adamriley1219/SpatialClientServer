#include "Engine/Core/Debug/DevConsole.hpp"

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

#include "Engine/Memory/Debug/Log.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/ActorRenderable.hpp"
#include "Game/PlayerController.hpp"
#include "Game/SpatialOSClient.hpp"

#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/AbilityBase.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/ActorBaseDefinition.hpp"
#include "Shared/Zone.hpp"
#include "Shared/AIController.hpp"
#include "Shared/SimController.hpp"

#include <vector>

#include <math.h>
//--------------------------------------------------------------------------
/**
* Game
*/
Game::Game()
{

}

//--------------------------------------------------------------------------
/**
* ~Game
*/
Game::~Game()
{

}

//--------------------------------------------------------------------------
/**
* GameStartup
*/
void Game::Startup()
{
	Logf( "Game", "startup" );
	m_DevColsoleCamera.SetOrthographicProjection( Vec2( -100.0f, -50.0f ), Vec2( 100.0f,  50.0f ) );
	m_DevColsoleCamera.SetModelMatrix( Matrix44::IDENTITY );

	Zone* zone = Zone::GetZone();
	zone->m_physics_system->SetGravity( Vec2::ZERO );
	
	LoadAbilities();
	LoadActors();

	m_curentCamera.SetModelMatrix( Matrix44::IDENTITY );
	m_curentCamera.SetOrthographicProjection( Vec2( -25.0f, -12.5f ), Vec2( 25.0f, 12.5f ) );	

	g_theEventSystem->SubscribeEventCallbackFunction( "API_connection_made", EventFunction( this, &Game::OnServerConnection ) );
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void Game::Shutdown()
{

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


//--------------------------------------------------------------------------
/**
* GameRender
*/
void Game::GameRender() const
{
	g_theRenderer->BindMaterial( g_theRenderer->CreateOrGetMaterialFromXML( "Data/Materials/default_unlit.mat" ) );

	std::vector<Vertex_PCU> verts;

	AddVertsForRing2D(verts, Vec2::ZERO, 5.0f, 0.5f, Rgba::CYAN);

	Vec2 client_pos = m_clientEntity->GetPosition();

	int int_pos_x = (int)client_pos.x;
	int int_pos_y = (int)client_pos.y;

	int offset_x = int_pos_x % 10;
	int offset_y = int_pos_y % 10;

	AABB2 box = AABB2::ONE_BY_ONE;
	box.AddPosition( int_pos_x - offset_x - 25, int_pos_y - offset_y - 25 );
	AddVertsForAABB2D( verts, box, Rgba::FADED_GRAY );

	box.AddPosition( 10, 0 );
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition( 10, 0 );
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition( 10, 0 );
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition( 10, 0 );
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition( 10, 0 );
	AddVertsForAABB2D( verts, box, Rgba::FADED_GRAY );

	box.AddPosition( 0, 10 );
	AddVertsForAABB2D( verts, box, Rgba::FADED_GRAY );

	box.AddPosition( -10, 0 );
	AddVertsForAABB2D( verts, box, Rgba::FADED_GRAY );
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(0, 10);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(0, 10);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(-10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(0, 10);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);
	box.AddPosition(10, 0);
	AddVertsForAABB2D(verts, box, Rgba::FADED_GRAY);

	g_theRenderer->DrawVertexArray(verts);

	Zone::GetZone()->m_physics_system->DebugRender(g_theRenderer, Rgba::GREEN);


	g_theDebugRenderSystem->RenderToCamera( &g_theGame->m_curentCamera );
}

//--------------------------------------------------------------------------
/**
* UpdateGame
*/
void Game::UpdateGame( float deltaSeconds )
{
	Zone::GetZone()->Update( deltaSeconds );
	
	// Can't go into zone because that's shared with server. Send updated client input here.
	SpatialOSClient::UpdatePlayerControls( m_clientEntity, m_clientController->GetMoveDirection() * m_clientEntity->GetSpeed() );

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
* AddEntity
*/
void Game::AddEntity( EntityBase* entity )
{
	switch ( entity->GetType() )
	{
	case (ENTITY_ABILITY):
		Zone::GetZone()->AddEntity( entity );
		break;
	case (ENTITY_ACTOR):
		Zone::GetZone()->AddEntityWithController( entity, ( (ActorBase*)entity )->GetController() );
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------------------
/**
* RemoveEntity
*/
void Game::RemoveEntity( EntityBase* entity )
{
	if( entity )
	{
		entity->Die();
	}
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
	m_curentCamera.SetModelMatrix( Matrix44::MakeTranslation2D( m_clientEntity->GetPosition() ) );
}

//--------------------------------------------------------------------------
/**
* OnServerConnection
*/
bool Game::OnServerConnection(EventArgs& args)
{
	UNUSED(args);
	// add player character
	m_clientController = new PlayerController();
	m_clientEntity = new ActorRenderable("player");
	m_clientEntity->Possess(m_clientController);

	SpatialOSClient::RequestEntityCreation( m_clientEntity );
	Logf( "Client", "Received connection from the server - made client entity" );
	return true;
}

//--------------------------------------------------------------------------
/**
* CreateSimulatedEntity
*/
EntityBase* Game::CreateSimulatedEntity( const std::string& name )
{
	
	if (AbilityBaseDefinition::DoesDefExist(name))
	{
		return new AbilityBase(name);
	}
	else if (ActorBaseDefinition::DoesDefExist(name))
	{
		ActorRenderable* actor = new ActorRenderable(name);
		if (name == "player")
		{
			actor->Possess(new SimController());
		}
		else
		{
			actor->Possess(new AIController());
		}
		return actor;
	}
	return nullptr;
}
