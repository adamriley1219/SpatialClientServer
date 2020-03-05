#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"

class ActorBase;
class EntityBase;
class PlayerController;
class Shader;

class WorldSim
{
	friend App;
public:
	WorldSim();
	~WorldSim();

	void Startup();
	void Shutdown();

	void UpdateWorldSim( float deltaSeconds );
	
	EntityBase* CreateSimulatedEntity( const std::string& name );

private:
	void ResetWorldSim();

private:
	bool m_isQuitting = false;
	
};