#include "Engine/Core/Debug/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec2.hpp"

#include "Server/WorldSim.hpp"
#include "Server/ServerCommon.hpp"
#include "Server/ServerApp.hpp"
#include "Server/SpatialOSServer.hpp"

#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/AbilityBase.hpp"
#include "Shared/Zone.hpp"
#include "Shared/AIController.hpp"
#include "Shared/SimController.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/ActorBaseDefinition.hpp"

#include <vector>

#include <math.h>
//--------------------------------------------------------------------------
/**
* WorldSim
*/
WorldSim::WorldSim()
{

}

//--------------------------------------------------------------------------
/**
* ~WorldSim
*/
WorldSim::~WorldSim()
{

}

//--------------------------------------------------------------------------
/**
* WorldSimStartup
*/
void WorldSim::Startup()
{
	Zone* zone = Zone::GetZone();
	zone->m_physics_system->SetGravity( Vec2::ZERO );
	
	std::cout << "Registering from XML" << std::endl;

	LoadAbilities();
	LoadActors();

	std::cout << "Finished registering" << std::endl;


	std::cout << "world setup" << std::endl;
	
	ActorBase* ai_actor = new ActorBase( "turret", Vec2( 6.5f, 7.0f ) );
	ai_actor->Possess( new AIController() );
	SpatialOSServer::RequestEntityCreation( ai_actor );

	ai_actor = new ActorBase( "turret", Vec2( 5.0f, 7.0f ) );
	ai_actor->Possess( new AIController() );
	SpatialOSServer::RequestEntityCreation( ai_actor );

	ai_actor = new ActorBase( "crawler", Vec2( -5.0f, 7.0f ) );
	ai_actor->Possess( new AIController() );
	SpatialOSServer::RequestEntityCreation( ai_actor );

	std::cout << "Finished world setup" << std::endl;
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void WorldSim::Shutdown()
{

}

//--------------------------------------------------------------------------
/**
* UpdateWorldSim
*/
void WorldSim::UpdateWorldSim( float deltaSeconds )
{
	Zone::UpdateZones( deltaSeconds );

	for( EntityBase* entity : Zone::GetZone()->m_entities )
	{
		if( entity )
		{
			SpatialOSServer::UpdatePosition( entity );
		}
	}
}

//--------------------------------------------------------------------------
/**
* CreateSimulatedEntity
*/
EntityBase* WorldSim::CreateSimulatedEntity( const std::string& name )
{
	if (AbilityBaseDefinition::DoesDefExist(name))
	{
		return new AbilityBase(name);
	}
	else if (ActorBaseDefinition::DoesDefExist(name))
	{
		ActorBase* actor = new ActorBase(name);
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

//--------------------------------------------------------------------------
/**
* ResetWorldSim
*/
void WorldSim::ResetWorldSim()
{
	Zone::GetZone()->Clear();
}
