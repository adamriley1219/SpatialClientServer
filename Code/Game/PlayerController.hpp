#pragma once

class Entity;


class PlayerController
{
public:
	PlayerController( Entity* toControl );
	~PlayerController();

	void Update( float deltaTime );

private:
	Entity* m_controlled = nullptr;

};
