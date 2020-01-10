#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"

class ActorBase;
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
	
private:
	void ResetWorldSim();

private:
	bool m_isQuitting = false;
	
};