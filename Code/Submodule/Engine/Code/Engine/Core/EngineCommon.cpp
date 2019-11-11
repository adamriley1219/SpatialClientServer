#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Strings/NamedStrings.hpp"



NamedStrings g_gameConfigBlackboard;


EventSystem* g_theEventSystem = nullptr;

DevConsole* g_theConsole = nullptr;

#ifdef PLATFORM_WINDOWS
DebugRenderSystem* g_theDebugRenderSystem = nullptr;
#endif // PLATFORM_WINDOWS

