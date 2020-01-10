#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"


class ActorRenderable;
class PlayerController;
class Shader;

class Game
{
	friend App;
public:
	Game();
	~Game();

	void Startup();
	void Shutdown();

	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );

	void GameRender() const;
	void UpdateGame( float deltaSeconds );

	void BeginCamera();
	void EndCamera();

	Camera* GetCamera();

private:
	void RenderDevConsole() const;

	void UpdateCamera( float deltaSeconds );

	bool OnServerConnection( EventArgs& args );

private:
	void ResetGame();

	// Helper Methods
	void ConstructGame();
	void DeconstructGame();
private:
	bool m_isQuitting = false;

	mutable Camera m_curentCamera;
	mutable Camera m_DevColsoleCamera;

	PlayerController* m_clientController = nullptr;
	ActorRenderable* m_clientEntity = nullptr;

};