#pragma once

#include <glm/glm.hpp>
#include <string>

using namespace glm;

struct Sprite {
    std::string filepath;
    int index;                        // Entity number this Sprite belongs to (zero-indexed)
    std::vector<int> frameDurations;  // duration of each page/frame for one cycle of the animation
    int frame;                        // current frame of animation this Sprite is on
    int framesPerRow;
    int framesPerCol;

    int frameWidth;                   // width of each frame in sprite sheet
    int frameHeight;                  // height of each frame in sprite sheet
};
