#pragma once

#include <glm/glm.hpp>
#include <string>

using namespace glm;

struct Sprite {
    std::string filepath;
    vec3 pos; // Position (before accounting for world-view-model matrix)
    int index; // start index in tex array
    int pages; // number of pages taken up by this spritesheet
    int pagesPerAnim; // pages taken up by each animation
    // (endPage = index + pages)
};
