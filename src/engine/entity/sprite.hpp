#pragma once

#include <glm/glm.hpp>
#include <string>

using namespace glm;

struct Sprite {
    std::string filepath;
    vec3 pos;                             // Position (before accounting for world-view-model matrix)
    uint8_t index;                        // Entity number this Sprite belongs to (zero-indexed)
    std::vector<int> frameDurations;      // duration of each page/frame for one cycle of the animation
    uint8_t frame;                        // current frame of animation this Sprite is on
    uint8_t framesPerRow;                 // number of frames per row, set by Renderer when registering Sprite
};
