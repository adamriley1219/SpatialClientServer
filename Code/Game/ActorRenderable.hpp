#pragma once
#include "Engine/Math/Vec2.hpp"

#include "Game/GameCommon.hpp"

#include "Shared/ActorBase.hpp"



class ActorRenderable : public ActorBase
{
public:
	ActorRenderable( const std::string& name );
	virtual ~ActorRenderable();

	virtual void Render() const;

	Rgba GetTint() const;


protected:
	

	Rgba m_tint;
};

