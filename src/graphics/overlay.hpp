#pragma once

#include "sprite.hpp"
#include <memory>

// Mildly experimental, using bitmask system to reduce if...else blocks or ternaries when setting type initially
enum OverlayType : uint8_t { // used to avoid excessive bool flags an if checks in code that handles overlays
    OVERLAY_TYPE_FLASHING        = 1,
    OVERLAY_TYPE_ANIMATED        = 1 << 1,
    OVERLAY_TYPE_FLASH_AND_ANIM  = OVERLAY_TYPE_FLASHING | OVERLAY_TYPE_ANIMATED,
    OVERLAY_TYPE_NONE            = 0
};

struct Flash {
    float startAlpha;
    float endAlpha;
    // originally, timePeriod was stored here, but better to keep computed dx in here, since timePeriod is only used to compute this
    float perFrameDx;
    float stretchScaleFactor;
    float amplitude;
    float x; // current x value
    float alpha; // current alpha value
};

struct OverlayDesc {
    // used to init a Sprite instance
    std::string PngFilepath;
    int         frameWidth;
    int         frameHeight;

    bool hasPeriodicFlash;
    // data used to init flash if present
    float startAlpha;
    float endAlpha;
    float perFrameDx;
    float stretchScaleFactor;
    float amplitude;
};

struct Overlay {
    std::shared_ptr<Sprite> sprite;
    float frameTimer;
    Flash flash;
};

// gets overlay from map, or pushes it to the map if it isn't already present
Overlay getOverlay(const std::string& name);

