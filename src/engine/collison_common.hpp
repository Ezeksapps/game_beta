#pragma once

#include <glm/glm.hpp>

using namespace glm;

struct AABB {
    vec3 min;
    vec3 max;
};

std::vector<uint32_t> getOccupiedCells(const AABB& bbox);
