#include "overlay.hpp"
#include "../fileio.hpp"

#include "json.hpp"

using json = nlohmann::json;

static constexpr char OVERLAYS_DIR[] = "assets/data/overlays/";

std::unordered_map<std::string, Overlay> overlayMap;

/*
 Overlays are not cached by the renderer (as only one sprite sheet exists), the entire array is uploaded when required
 */

// load and create an overlay from JSON
Overlay loadOverlay(const std::string& name) {
    json overlayJson = readJsonAsset(std::string(OVERLAYS_DIR + name + ".json").c_str());

    Sprite sprite;
    sprite.filepath = overlayJson["sprite"], sprite.frameWidth = overlayJson["frameWidth"], sprite.frameHeight = overlayJson["frameHeight"];

    if (overlayJson["hasPeriodicFlash"].get<bool>()) {
        return Overlay {
            .sprite = sprite, .frameTimer = 0.0f,
            .flash = Flash {
                .startAlpha = overlayJson["startAlpha"],
                .endAlpha   = overlayJson["endAlpha"],
                .timePeriod = overlayJson["timePeriod"]
            }
        };
    }

    return Overlay { .sprite = sprite, .frameTimer = 0.0f, };
}

Overlay& getOverlay(const std::string& name) {
    if (overlayMap.contains(name)) return overlayMap.at(name);
    else {
        overlayMap.emplace(name, loadOverlay(name));
        return overlayMap.at(name);
    }
}
