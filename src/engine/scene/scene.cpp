#include "scene.hpp"

#include <cstdint>
#include <filesystem>
#include <stdexcept>

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

    genSceneSptlHash();
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

void Scene::genSceneQuadtree() {
    // TODO
}

// check if two AABBs intersect
// source: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
bool intersect(const AABB& a, const AABB& b) {
    return (
        a.min.x <= b.max.x &&
        a.max.x >= b.min.x &&
        a.min.y <= b.max.y &&
        a.max.y >= b.min.y
    );
}

void Scene::resolveCollisions() {
    /* To avoid the inefficiency that comes with checking every possible pair of entities for collisions,
     * I'm instead using a spatial hash system, that will only perform collision tests on pairs of entities
     * that are close enough to each other that a collision might be possible. For collisions between entities and
     * the 3D scene, a quadtree is used for this same purpose
     */
}

// CHANGE: AABBs now 3D again


uint32_t getPackedCoord(float x, float y) {
    // find the grid coordinates the input x and y coords correspond to, grids are 4x4
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

void Scene::genSceneSptlHash() {
    for (const std::shared_ptr<Entity>& entity : m_pEntities) {
        vec3 pos = entity->m_pos; // entity centre pos
        // TODO: The size of geometry might eventually differ between entities, this must be handled! current size is const 1x1
        AABB bbox {
            .min = vec3(pos.x - 0.5, pos.y - 0.5, 0.0f),
            .max = vec3(pos.x + 0.5, pos.y + 0.5, 0.0f /* HANDLE Z */) // entity should be snapped to floor below them in z
        };

        uint32_t minCoords = getPackedCoord(bbox.min.x, bbox.min.y);
        uint32_t maxCoords = getPackedCoord(bbox.max.x, bbox.max.y);

        // if the min and max points of the entity are not in the same cell, then this entity's geometry spans multiple cells
        if (minCoords != maxCoords) {
            // TODO: check other two corners, entity might be occupying up to 4 cells simulatenously, not just 2
            m_sptlHash[minCoords].push_back(entity);
            m_sptlHash[maxCoords].push_back(entity);
        }
        // else entity is only in one cell
        else m_sptlHash[getPackedCoord(pos.x, pos.y)].push_back(entity);
    }
}
