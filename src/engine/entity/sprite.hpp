#pragma once

#include <glm/glm.hpp>
#include <string>
#include <chrono>

using namespace glm;

struct Sprite {
    std::string filepath;
    vec3 pos;                             // Position (before accounting for world-view-model matrix)
    uint8_t index;                        // Entity number this Sprite belongs to (zero-indexed)
    std::vector<int> frameDurations;      // duration of each page/frame for one cycle of the animation
    std::chrono::steady_clock animClock;  // used to time this Sprite's animations
    uint8_t frame;                        // current frame of animation this Sprite is on
    // -- UNUSED -- //
    //int pagesPerAnim; // pages taken up by each animation
    //int frames; // number of pages taken up by this spritesheet
};
