#pragma once

class ActorRenderable;

struct Vec2;
struct IntVec2;
struct Ray3;


class PlayerController
{
public:
	PlayerController( ActorRenderable* toControl );
	~PlayerController();

	void Update( float deltaTime );

private:
	Vec2 GetScreenMousePos();
	IntVec2 GetRawScreenMousePos();

private:
	ActorRenderable* m_controlled = nullptr;

};
