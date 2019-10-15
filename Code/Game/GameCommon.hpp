#pragma once
#include "Shared/SharedCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex/Vertex_PCU.hpp"
#include "Game/GameUtils.hpp"

class RenderContext; // All may know about the renderer and the globe. You must include to use it.
extern RenderContext* g_theRenderer;

class AudioSystem;
extern AudioSystem* g_theAudioSystem;

class WindowContext;
extern WindowContext* g_theWindowContext;

class RNG;
extern RNG* g_theRNG;

class Game;
extern Game* g_theGame;

class App;
extern App* g_theApp;

class InputSystem;
extern InputSystem* g_theInputSystem;

class WindowContext;
extern WindowContext* g_theWindowContext;

extern bool g_isInDebug;

//--------------------------------------------------------------------------
// Constant global variables.
//--------------------------------------------------------------------------
constexpr float WORLD_WIDTH = 50.f;
constexpr float WORLD_HEIGHT = 25.f;
constexpr float WORLD_CENTER_X = WORLD_WIDTH / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_HEIGHT / 2.f;


