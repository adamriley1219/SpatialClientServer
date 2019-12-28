#include "Shared/AIController.hpp"
#include "Shared/ActorBase.hpp"
#include "Shared/Zone.hpp"

#include "Engine/Core/Time/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"

//--------------------------------------------------------------------------
/**
* AIController
*/
AIController::AIController()
	: ControllerBase()
{
	m_attack_timer = new StopWatch( &Clock::Master );
	m_attack_timer->SetAndReset( 1.0f );
}

//--------------------------------------------------------------------------
/**
* ~AIController
*/
AIController::~AIController()
{
	SAFE_DELETE( m_attack_timer );
}

//--------------------------------------------------------------------------
/**
* Update
*/
void AIController::Update(float deltaTime)
{
	UNUSED(deltaTime);
	ActorBase* player = FindClosestPlayer();

	if( player )
	{
		MoveTowardPlayer( player );
		AttackPlayerIfCan( player );
	}

}

//--------------------------------------------------------------------------
/**
* FindClosestPlayer
*/
ActorBase* AIController::FindClosestPlayer()
{
	Zone* zone = Zone::GetZone();
	if( zone )
	{
		float closestDist = m_searchRange;
		EntityBase* toRet = nullptr;
		for( EntityBase* entity : zone->m_entities )
		{
			if( entity && entity->GetName() == "player" )
			{
				Vec2 disp = entity->GetPosition() - m_controlled->GetPosition();
				float dist = disp.GetLength();
				if( dist < closestDist )
				{
					toRet = entity;
					closestDist = dist;
				}
			}
		}
		return ( ActorBase* ) toRet;
	}
	return nullptr;
}

//--------------------------------------------------------------------------
/**
* MoveTowardPlayer
*/
void AIController::MoveTowardPlayer( ActorBase* player )
{
	Vec2 disp = player->GetPosition() - m_controlled->GetPosition();
	disp.Normalize();

	m_controlled->ApplyForce( disp * m_controlled->GetSpeed() );
}

//--------------------------------------------------------------------------
/**
* AttackPlayerIfCan
*/
void AIController::AttackPlayerIfCan( ActorBase* player )
{
	if( m_attack_timer->HasElapsed() )
	{
		m_controlled->PreformAbility( "basic_attack", player->GetPosition() );

		m_attack_timer->Reset();
	}
}
