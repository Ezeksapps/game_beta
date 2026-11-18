#pragma once

#include "sprite.hpp"
#include <optional>
#include <memory>

struct Flash {
    float startAlpha;
    float endAlpha;
    int timePeriod;
};

struct Overlay {
    // NOTE: The overlay animation doesn't need to sync to anything, it can be played as-is
    // overlays are defined through JSON and loaded when needed
    std::shared_ptr<Sprite> sprite;
    float frameTimer;
    std::optional<Flash> flash;
};

// gets overlay from map, or pushes it to the map if it isn't already present
Overlay& getOverlay(const std::string& name);

