#pragma once
// #include "Engine/Core/EventSystem.hpp"
// 
// class Clock;
// 
// //--------------------------------------------------------------------------
// class ServerApp
// {
// public:
// 	ServerApp() {};
// 	~ServerApp() {};
// 	void Startup();
// 	void Shutdown();
// 	void RunFrame();
// 	
// 	void HandleQuitRequested();
// 	bool IsQuitting() const { return m_isQuitting; }
// 
// 	static bool QuitEvent( EventArgs& args );
// 
// private:
// 	void BeginFrame();
// 	void Update( float deltaSeconds );
// 	void EndFrame();
// 	void RegisterEvents();
// 
// private:
// 	bool m_isQuitting = false;
// 	bool m_isPaused = false;
// 	bool m_isSlowMo = false;
// 	bool m_isFastMo = false;
// 	float m_consoleTextHeight = 2.0f;
// 
// 	Clock* m_gameClock = nullptr;
// 
// };
