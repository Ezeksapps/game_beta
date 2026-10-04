#pragma once

#include <glm/glm.hpp>



using namespace glm;

struct AABB {
    vec3 min;
    vec3 max;
};

struct OccupiedCells {
    std::array<uint32_t, 4> cells;
    uint8_t count;
};

OccupiedCells getOccupiedCells(const AABB& bbox);
