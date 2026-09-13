#include "scene.hpp"
#include <algorithm>

/* Definitions for member functions of the Quadtree struct */

/*
Box2D genBounds(std::array<vec2, 4>::iterator begin, std::array<vec2, 4>::iterator end) {
    Box2D result;
    for (auto it = begin; it != end; ++it)
        result.includeVertex(*it);
    return result;
}*/

vec2 getMidpoint(const vec2& vertex1, const vec2& vertex2) {
    return { (vertex1.x + vertex2.x) / 2.f, (vertex1.y + vertex2.y) / 2.f };
}

uint16_t buildNode(Quadtree& tree, const AABB& bounds, std::vector<AABB>::iterator begin, std::vector<AABB>::iterator end) {
    // if all AABBs have been iterated over already, cancel operation
    if (begin == end) return nullNode;

    // index of this new node = size of vec of exisitng nodes
    uint16_t nodeIndex = tree.nodes.size();
    // set the starting index of the AABBs which correspond to this node
    tree.nodeBBoxesBegin[nodeIndex] = (begin - tree.bboxes.begin());
    // add the new empty node to the nodes vec, data will now be subsequently filled in
    tree.nodes.emplace_back();

    vec2 center = getMidpoint(bounds.min, bounds.max);;

    // region below centre
    auto bottom = [center, vec2(*getMidpoint)](const AABB& bbox){ return getMidpoint(bbox.min, bbox.max).y < center.y; };
    // left region, where centre splits the bbox in half
    auto left   = [center, vec2(*getMidpoint)](const AABB& bbox){ return getMidpoint(bbox.min, bbox.max).x < center.x; };

    /* get iterators for... */
    // Lower two quadrants
    std::vector<AABB>::iterator split_y       = std::partition(begin, end, bottom);
    // Bottom-left quadrant
    std::vector<AABB>::iterator split_x_lower = std::partition(begin, split_y, left);
    // Bottom-right quadrant
    std::vector<AABB>::iterator split_x_upper = std::partition(split_y, end, left);

    /* Create new node at... */
    // top-left quadrant
    tree.nodes[nodeIndex].children[0][0] = buildNode(tree, { bounds.min, center }, begin, split_x_lower);
    // top-right quadrant
    tree.nodes[nodeIndex].children[0][1] = buildNode(tree, { { center.x, bounds.min.y }, { bounds.max.x, center.y } }, split_x_lower, split_y);
    // bottom-left quadrant
    tree.nodes[nodeIndex].children[1][0] = buildNode(tree, { { bounds.min.x, center.y }, { center.x, bounds.max.y } }, split_y, split_x_upper);
    // bottom-right quadrant
    tree.nodes[nodeIndex].children[1][1] = buildNode(tree, { center, bounds.max }, split_x_upper, end);

    return nodeIndex;
}

void Quadtree::build(std::vector<AABB>& bboxesIn) {
    // preserve single source of truth, transfer all bboxes to struct
    bboxes = std::move(bboxesIn);
    // build full tree
    root = buildNode(*this, bbox, bboxes.begin(), bboxes.end());
    nodeBBoxesBegin.push_back(bboxes.size());
}
