#pragma once
#include "Engine/Math/Vec2.hpp"

#include "Game/GameCommon.hpp"

#include "Shared/EntityBase.hpp"

class Entity : public EntityBase
{
public:
	Entity();
	virtual ~Entity();

	virtual void Render() const;

	Rgba GetTint() const;


protected:
	

	Rgba m_tint;
};

