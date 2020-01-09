#pragma once
#include "Shared/ControllerBase.hpp"
#include "Engine/Math/Vec2.hpp"

class ActorBase;

struct Vec2;
struct IntVec2;
struct Ray3;


class PlayerController : public ControllerBase
{
public:
	PlayerController();
	~PlayerController();

	virtual void Update( float deltaTime );
	const Vec2& GetMoveDirection() const;

private:
	Vec2 GetScreenMousePos();
	IntVec2 GetRawScreenMousePos();

	Vec2 m_moveDir = Vec2::ZERO;

};
