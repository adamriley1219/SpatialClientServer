#pragma once
#include "Game/Game.hpp"

#include "Engine/Core/EventSystem.hpp"

class Clock;

//--------------------------------------------------------------------------
class App
{
public:
	App() {};
	~App() {};
	void Startup();
	void Shutdown();
	void RunFrame();

	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleCharPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool HandleQuitRequested();

	static bool QuitEvent( EventArgs& args );

	bool IsPaused() const;
	void Unpause();
	void Pause();
	void TogglePause();
	float GetGlobleTime() const;
private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
	void ToggleDebug();
	void RegisterEvents();

private:
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	float m_consoleTextHeight = 2.0f;

	Clock* m_gameClock = nullptr;

};
