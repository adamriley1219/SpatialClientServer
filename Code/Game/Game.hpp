#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"


class ActorRenderable;
class EntityBase;
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

	ActorRenderable* GetPlayerEntity() { return m_clientEntity; }
	
public:
	void AddEntity( EntityBase* entity );
	void RemoveEntity( EntityBase* entity );

	EntityBase* CreateSimulatedEntity( const std::string& name );
private:
	void RenderDevConsole() const;

	void UpdateCamera( float deltaSeconds );

	bool OnServerConnection( EventArgs& args );

private:
	bool m_isQuitting = false;

	mutable Camera m_curentCamera;
	mutable Camera m_DevColsoleCamera;

	PlayerController* m_clientController = nullptr;
	ActorRenderable* m_clientEntity = nullptr;



};