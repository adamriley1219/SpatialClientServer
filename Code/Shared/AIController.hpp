#pragma once
#include "Shared/ControllerBase.hpp"

class StopWatch;

class AIController : public ControllerBase
{
public:
	AIController();
	virtual ~AIController();

	virtual void Update( float deltaTime );

protected:
	ActorBase* FindClosestPlayer();
	void MoveTowardPlayer( ActorBase* player );
	void AttackPlayerIfCan( ActorBase* player );

protected:
	StopWatch* m_attack_timer;
	float m_searchRange = 5.0f;

private:
	float m_current_deltatime = 0.0f;

};