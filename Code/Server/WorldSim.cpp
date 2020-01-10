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
#include "Server/SelfSubActor.hpp"
#include "Server/SpatialOSServer.hpp"

#include "Shared/AbilityBaseDefinition.hpp"
#include "Shared/Zone.hpp"
#include "Shared/AIController.hpp"
#include "Shared/ActorBase.hpp"

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
	
	SelfSubActor* turret = new SelfSubActor( "turret", Vec2( 6.5f, 7.0f ) );
	turret->Possess( new AIController() );

	turret = new SelfSubActor( "turret", Vec2( 5.0f, 7.0f ) );
	turret->Possess( new AIController() );

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
	std::cout << "begin sending updates" << std::endl;
	uint count = 0;
	for( EntityBase* entity : Zone::GetZone()->m_entities )
	{
		std::cout << "Iteration: " << count++ << std::endl;
		if( entity )
		{
			SpatialOSServer::UpdatePosition( entity );
		}
	}
	std::cout << "end sending updates" << std::endl;
}

//--------------------------------------------------------------------------
/**
* ResetWorldSim
*/
void WorldSim::ResetWorldSim()
{
	Zone::GetZone()->Clear();
}
