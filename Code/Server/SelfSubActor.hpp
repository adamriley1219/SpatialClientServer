#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Shared/ActorBase.hpp"

class ControllerBase;

class SelfSubActor : public ActorBase
{
public:
	SelfSubActor( const std::string& name );
	SelfSubActor( const std::string& name, const Vec2& position );
	virtual ~SelfSubActor();

};