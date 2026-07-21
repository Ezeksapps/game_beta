#pragma once

#include <glm/glm.hpp>
#include <string>

using namespace glm;

struct Sprite {
    std::string filepath;
    int index;                        // Entity number this Sprite belongs to (zero-indexed)
    std::vector<int> frameDurations;  // duration of each page/frame for one cycle of the animation
    int frame;                        // current frame of animation this Sprite is on
    int framesPerRow;                 // number of frames per row, set by Renderer when registering Sprite
};
