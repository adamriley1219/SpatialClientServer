#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Shared/EntityBase.hpp"

class ActorBase;

class AbilityBase : public EntityBase
{
public:
	AbilityBase(const std::string& name, uint zone_id);
	virtual ~AbilityBase();

	virtual void Update( float deltaSeconds );
	
	void SetDirection( const Vec2& dir );
	void SetOwner( ActorBase* owner );

	float m_life_time = 0.1f;

protected:
	float m_time_alive = 0.0f;
	float m_speed = 0.0f;
	Vec2 m_direction = Vec2::ZERO;

	ActorBase* m_owner = nullptr;

};