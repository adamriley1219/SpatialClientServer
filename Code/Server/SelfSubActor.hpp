#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Shared/ActorBase.hpp"

class ControllerBase;

class SelfSubBase : public ActorBase
{
public:
	SelfSubBase( const std::string& name );
	SelfSubBase( const std::string& name, const Vec2& position );
	virtual ~SelfSubBase();

};