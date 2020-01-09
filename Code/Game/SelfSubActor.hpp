#pragma once
#include "Engine/Math/Vec2.hpp"

#include "Game/GameCommon.hpp"

#include "Shared/ActorBase.hpp"



class SelfSubActor: public ActorBase
{
public:
	SelfSubActor( const std::string& name );
	SelfSubActor(const std::string& name, const Vec2& position);
	virtual ~SelfSubActor();
};

