#pragma once
#include "Shared/ControllerBase.hpp"

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

private:
	Vec2 GetScreenMousePos();
	IntVec2 GetRawScreenMousePos();

};
