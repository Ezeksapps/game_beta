#include "collison_common.hpp"

uint32_t getPackedCoord(float x, float y) {
    // find the grid coordinates the input x and y coords correspond to, grids are 4x4
    // note, any negative coords will wrap over to the 16-bit uint limit, and will still work here
    uint16_t row = std::floor(y / 4);
    uint16_t col = std::floor(x / 4);

    uint32_t packed = 0;
    // order will be column, then row, so that we have xy instead of yx

    // set first 16 bits to val of col
    packed = col;
    // set upper 16 bits to val of row
    packed |= static_cast<uint32_t>(row << 16);

    return packed;
}

std::vector<uint32_t> getOccupiedCells(const AABB& bbox) {
    /*
     *      If minX and maxX's floored vals differ, then then the AABB occupies >= 2 cells
     *      If minY and maxY's floored vals also differ, then the AABB mut occupy 2<x<5 cells
     *      and vice versa
     */
    int16_t minX = std::floor(bbox.min.x / 4);
    int16_t maxX = std::floor(bbox.max.x / 4);
    int16_t minY = std::floor(bbox.min.y / 4);
    int16_t maxY = std::floor(bbox.max.y / 4);

    uint32_t minCoords = getPackedCoord(bbox.min.x, bbox.min.y);
    uint32_t maxCoords = getPackedCoord(bbox.max.x, bbox.max.y);

    // if the min and max points of the entity are not in the same cell, then this entity's geometry spans multiple cells
    if (minCoords != maxCoords) {
        // if either of the two min/max coordinates are in the same cell, then this entity is in two cells
        if (minX == maxX || minY == maxY) return { minCoords, maxCoords };
        // otherwise, entity must be in four cells (there is no possibility for three-cell intersections)
        else return {
            getPackedCoord(bbox.min.x, bbox.min.y),  // min corner
            getPackedCoord(bbox.max.x, bbox.min.y),  // (max x, min y)
            getPackedCoord(bbox.min.x, bbox.max.y),  // (min x, max y)
            getPackedCoord(bbox.max.x, bbox.max.y)   // max corner
        };
    }
    // else entity is only in one cell, return minCoords (maxCoords would also work, they're the same)
    else return { minCoords };
}
