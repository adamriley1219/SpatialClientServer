#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Shared/EntityBase.hpp"

class ControllerBase;

class ActorBase : public EntityBase
{
public:
	ActorBase( const std::string& name );
	ActorBase( const std::string& name, const Vec2& position );
	virtual ~ActorBase();

	virtual void Update( float deltaTime );

public:
	void PreformAbility( const std::string& ability_name, const Vec2& target_position );
	bool Possess( ControllerBase* controller );

	ControllerBase* GetController() const;

	float GetSpeed() const;

protected:
	void BasicAttack( const Vec2& input_position );

protected:
	std::string m_basic_attack = "none";
	bool m_possessable = false;
	ControllerBase* m_owner = nullptr;

	float m_speed = 0.0f;

private:
	// Used through creation
	void DefineThroughName( const std::string& name );

};