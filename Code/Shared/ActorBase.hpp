#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Shared/EntityBase.hpp"



class ActorBase : public EntityBase
{
public:
	ActorBase( const std::string& name, uint zone_id );
	virtual ~ActorBase();

public:
	void PreformAbility( const std::string& ability_name, const Vec2& input_position );

protected:
	void BasicAttack( const Vec2& input_position );

protected:
	std::string m_basic_attack = "none";

};