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

	// Helper Methods
	void ConstructWorldSim();
	void DeconstructWorldSim();
private:
	bool m_isQuitting = false;
	
	std::vector<ActorBase*> m_entities;

};