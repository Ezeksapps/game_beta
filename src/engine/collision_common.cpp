#include "collison_common.hpp"

uint32_t packCell(int16_t gridX, int16_t gridY) { // input coords must already be in grid space
    return (static_cast<uint16_t>(gridX) << 16 | static_cast<uint16_t>(gridY));
}

uint32_t getPackedCoord(float x, float y) {
    return packCell(static_cast<int16_t>(std::floor(x / 4)), static_cast<int16_t>(std::floor(y / 4)));
}

OccupiedCells getOccupiedCells(const AABB& bbox) {

    const int16_t minX = std::floor(bbox.min.x / 4);
    const int16_t maxX = std::floor(bbox.max.x / 4);
    const int16_t minY = std::floor(bbox.min.y / 4);
    const int16_t maxY = std::floor(bbox.max.y / 4);

    const uint32_t minCell = packCell(minX, minY);

    if (minX == maxX && minY == maxY) return {{ minCell }, 1};

    const uint32_t maxCell = packCell(maxX, maxY);

    if (minX == maxX || minY == maxY) return {{ minCell, maxCell }, 2};

    return {{
        minCell,
        packCell(maxX, minY),
        packCell(minX, maxY),
        maxCell
    }, 4};
}
