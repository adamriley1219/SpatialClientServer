#pragma once
#include "Shared/ControllerBase.hpp"
#include "Engine/Math/Vec2.hpp"

class ActorBase;

struct Vec2;
struct IntVec2;
struct Ray3;


class SimController : public ControllerBase
{
public:
	SimController();
	~SimController();

	virtual void Update( float deltaTime );
	void SetMoveDirection( const Vec2& direction );

private:
	Vec2 m_moveDir = Vec2::ZERO;

};
