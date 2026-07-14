#pragma once

/* Handles game systems that are singleton, yet need to be shared across classes. For example, the Renderer */

#include "graphics/renderer.hpp"

struct GameState {
    std::unique_ptr<Renderer> g_renderer;
};
