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
	ConstructWorldSim();
}

//--------------------------------------------------------------------------
/**
* ~WorldSim
*/
WorldSim::~WorldSim()
{
	DeconstructWorldSim();
}

//--------------------------------------------------------------------------
/**
* WorldSimStartup
*/
void WorldSim::Startup()
{
	Zone* zone = Zone::AddZone( 0 );
	zone->m_physics_system->SetGravity( Vec2::ZERO );
	
	LoadAbilities();
	LoadActors();
}

//--------------------------------------------------------------------------
/**
* Shutdown
*/
void WorldSim::Shutdown()
{
	Zone::ClearAllZones();
}

//--------------------------------------------------------------------------
/**
* UpdateWorldSim
*/
void WorldSim::UpdateWorldSim( float deltaSeconds )
{
	Zone::UpdateZones( deltaSeconds );
}

//--------------------------------------------------------------------------
/**
* ResetWorldSim
*/
void WorldSim::ResetWorldSim()
{
	DeconstructWorldSim();
	ConstructWorldSim();
}

//--------------------------------------------------------------------------
/**
* ConstructWorldSim
*/
void WorldSim::ConstructWorldSim()
{

}

//--------------------------------------------------------------------------
/**
* DeconstructWorldSim
*/
void WorldSim::DeconstructWorldSim()
{

}
