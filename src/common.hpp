#pragma once

#include <memory>

struct GameState; // forward declaration, defined by entry point which should include game_state.hpp

/* Global game state manager */
extern std::shared_ptr<GameState> g_state;

/* Defines global macros required by libs */

#if defined(__linux__)

#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#define PLATFORM_LINUX 1

#elif defined (_WIN32)

#define GLFW_EXPOSE_NATIVE_WIN32
#define PLATFORM_WIN32 1

#endif
