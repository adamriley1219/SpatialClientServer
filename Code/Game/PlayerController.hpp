#pragma once

class ActorRenderable;


class PlayerController
{
public:
	PlayerController( ActorRenderable* toControl );
	~PlayerController();

	void Update( float deltaTime );

private:
	ActorRenderable* m_controlled = nullptr;

};
