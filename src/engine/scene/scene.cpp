#include "scene.hpp"

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <iostream>

#include "json.hpp"
#include "../fileio.hpp"

#include "../entity/entity.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

Scene::Scene(const std::string& sceneDir) {

    // do not accept bad paths
    if (!fs::is_directory(sceneDir)) throw std::runtime_error("Scene constructor was provided with a non-directory");
    fs::path dir = sceneDir;

    // find the two needed files (The scene JSON and glTF and validate directory)
    fs::path jsonFilepath, gltfFilepath;
    int jsonCounter = 0, gltfCounter = 0;
    for (const fs::directory_entry& file : fs::recursive_directory_iterator(dir)) {

        if (jsonCounter > 1 || gltfCounter > 1) throw std::runtime_error("Scene directory contains more than one JSON or glTF file");
        if (!fs::is_regular_file(file)) continue; // ignore any non-standard files

        if (file.path().extension() == ".json") {
            ++jsonCounter;
            jsonFilepath = file.path();
        }
        if (file.path().extension() == ".glb") {
            ++gltfCounter;
            gltfFilepath = file.path();
        }
    }

    // JSON and glTF must have the same name
    if (gltfFilepath.stem() != jsonFilepath.stem()) throw std::runtime_error("Scene JSON and glTF filenames do not match");

    m_glbFilepath = gltfFilepath;
    loadSceneJson(jsonFilepath);
}

Scene::~Scene() {}

void Scene::loadSceneJson(const std::string& sceneFilepath) {
    const char* sceneJson = readJsonAsset(sceneFilepath.c_str());

    //json exits = json::parse(sceneJson)["exits"];
    json entities = json::parse(sceneJson)["entities"];

    // entities is an array of objects
    int entityIndex = 0;
    for (const json& entity : entities) {
        /* Entity format:
         * "type":    <name of entity subclass>
         * "sprites": <rel filepath to sprite sheet JSON>
         * "json":    <rel filepath to entity info/behaviour JSON>
         * "posX":    <position in x-axis>
         * "posY":    <position in y-axis>
         */

        // should be able to create, based on 'type' an object of Entity's subclasses,
        // for example  if type = "p", then make shared Pokemon
        std::shared_ptr<Entity> pEntity = std::make_shared<Entity>(entity["json"], entity["sprites"], entityIndex);
        pEntity->m_pos = vec3(entity["posX"], entity["posY"], 0.5f);
        m_pEntities.push_back(pEntity);
        ++entityIndex;
    }
    // TODO: Define scene JSON format
}

/* -- INDEV -- //

Box2D genBounds(std::array<vec2, 4>::iterator begin, std::array<vec2, 4>::iterator end) {
    Box2D result;
    for (auto it = begin; it != end; ++it)
        result.includeVertex(*it);
    return result;
}

vec2 getMidpoint(const vec2& vertex1, const vec2& vertex2) {
    return { (vertex1.x + vertex2.x) / 2.f, (vertex1.y + vertex2.y) / 2.f };
}

uint32_t build_impl(Quadtree& tree, const Box2D& bounds, std::vector<AABB>::iterator begin, std::vector<AABB>::iterator end) {
    if (begin == end) return nullNode;

    uint16_t result = tree.nodes.size();
    tree.nodes.emplace_back();

    //if (std::equal(begin + 1, end, begin)) return result;

    vec2 center = getMidpoint(bounds.min, bounds.max);

    auto bottom = [center](const vec2& vertex){ return vertex.y < center.y; };
    auto left   = [center](const vec2& vertex){ return vertex.x < center.x; };

    std::vector<AABB>::iterator split_y = std::partition(begin, end, bottom);
    std::vector<AABB>::iterator split_x_lower = std::partition(begin, split_y, left);
    std::vector<AABB>::iterator split_x_upper = std::partition(split_y, end, left);

    tree.nodes[result].children[0][0] = build_impl(tree, { bounds.min, center }, begin, split_x_lower);
    tree.nodes[result].children[0][1] = build_impl(tree, { { center.x, bounds.min.y }, { bounds.max.x, center.y } }, split_x_lower, split_y);
    tree.nodes[result].children[1][0] = build_impl(tree, { { bounds.min.x, center.y }, { center.x, bounds.max.y } }, split_y, split_x_upper);
    tree.nodes[result].children[1][1] = build_impl(tree, { center, bounds.max }, split_x_upper, end);

    return result;
}

Quadtree build(std::array<vec2, 4>& sceneBounds, std::vector<AABB>& sceneBboxes)
{
    Quadtree result;
    result.root = build_impl(result, genBounds(sceneBounds.begin(), sceneBounds.end()), sceneBBoxes.begin, sceneBboxes.end);
    return result;
}*/


void Scene::genSceneQuadtree() {
    //m_sceneQuadtree = build(m_sceneBounds, m_sceneBboxes);
}


// check if two AABBs intersect
// source: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
bool intersect(const AABB& a, const AABB& b) {
    return (
        a.min.x <= b.max.x &&
        a.max.x >= b.min.x &&
        a.min.y <= b.max.y &&
        a.max.y >= b.min.y &&
        a.min.z <= b.max.z &&
        a.max.z >= b.min.z
    );
}

void Scene::resolveCollisions() {
    /* To avoid the inefficiency that comes with checking every possible pair of entities for collisions,
     * I'm instead using a spatial hash system, that will only perform collision tests on pairs of entities
     * that are close enough to each other that a collision might be possible. For collisions between entities and
     * the 3D scene, a quadtree is used for this same purpose
     */


}
