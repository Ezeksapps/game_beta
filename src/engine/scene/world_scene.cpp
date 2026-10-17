#include "world_scene.hpp"
#include "../ui/dialog.hpp"

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <iostream>

#include "json.hpp"
#include "../fileio.hpp"

#include "../entity/entity.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

WorldScene::WorldScene(const std::string& sceneDir, std::shared_ptr<Entity> player) : Scene(sceneDir, player) {

    setupEntityCallbacks(0); // setup callbacks for player entity

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

WorldScene::~WorldScene() {}

void WorldScene::setupEntityCallbacks(const int& entityIndex) {
    m_pEntities[entityIndex]->setStartMovementCallback([this](const int& index, vec3 translVec) {
        m_translsMap.emplace(index, translVec);
    });

    m_pEntities[entityIndex]->setChangeMovementDirectionCallback([this](const int& index, vec3 translVec) {
        m_translsMap[index] = translVec; // update translation vector for this entity
    });

    m_pEntities[entityIndex]->setEndMovementCallback([this](const int& index) {
        m_translsMap.erase(index); // erase translation vector at index
    });
}

void WorldScene::loadSceneJson(const std::string& sceneFilepath) {
    const char* sceneJson = readJsonAsset(sceneFilepath.c_str());

    //json exits = json::parse(sceneJson)["exits"];
    json entities = json::parse(sceneJson)["entities"];

    // entities is an array of objects, start count from 1, slot 0 reserved for player
    int entityIndex = 1;
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
        setupEntityCallbacks(entityIndex);

        ++entityIndex;
    }
    // TODO: Define scene JSON format
}

float getIntersectArea(const AABB& a, const AABB& b) {
    // Find the overlapping rectangle on the X axis
    float overlapMinX = std::max(a.min.x, b.min.x);
    float overlapMaxX = std::min(a.max.x, b.max.x);

    // Find the overlapping rectangle on the Y axis
    float overlapMinY = std::max(a.min.y, b.min.y);
    float overlapMaxY = std::min(a.max.y, b.max.y);

    // do not test for no overlap, if this func has been executed, then there must be overlap

    // The area of the intersection rectangle
    return (overlapMaxX - overlapMinX) * (overlapMaxY - overlapMinY);
}

// a translation should be rejected if it would cause bbox A to further overlap with bbox B, assuming they are already overlapping
// of course, the entity should still be able to move in the other directions which would not cause this
// for diagonals, you can check either x or y, because both direction's translVecs are applied simultaneously
bool rejectTransl(const AABB& a, const AABB& b, const vec3& translVec, const Direction& direction) {

    // the AABB that a would become if the translation were not rejected
    AABB translBbox = a;
    translBbox.min += translVec;
    translBbox.max += translVec;

    const float allowance = 0.01f;
    return getIntersectArea(translBbox, b) > (getIntersectArea(a, b) + allowance);
}

void WorldScene::update() {

    std::vector<uint8_t> translsToTerminate = {};

    // update entity positions, including sptl hash positions
    if (!m_translsMap.empty()) {
        for (const auto& [index, translVec] : m_translsMap) {

            const std::shared_ptr<Entity>& entity = m_pEntities[index];

            std::vector<uint8_t>& knownCollisions = entity->m_knownCollisions;

            /* Do not perform the translation yet, first test if the translation should be allowed.
             * For example, the translation should be rejected if it would lead to further overlapping
             * an entity that this entity is already colliding with
             */
            bool reject = false;
            if (!entity->m_knownCollisions.empty()) {
                for (uint8_t i = 0; i < entity->m_knownCollisions.size(); ++i) {
                    AABB b = m_pEntities[entity->m_knownCollisions[i]]->getAABB();
                    AABB a = entity->getAABB();
                    reject = rejectTransl(a, b, translVec, entity->m_direction);
                }
            }
            if (reject == true) {
                // NOTE: don't run endMovement in this loop, because then that changes the indices in the transl map
                // and since the program is actively iterating that map, a segfault will occur
                translsToTerminate.push_back(index);
                continue;
            }
            else entity->m_pos += translVec;

            OccupiedCells prevOccupiedCells = getOccupiedCells(entity->getAABB());
            OccupiedCells newOccupiedCells = getOccupiedCells(entity->getAABB());

            // should remove entity index from any cells which it is no longer in and insert it into any newly occupied cells
            // TODO: this can definitely be optimised somehow...
            for (uint8_t i = 0; i < prevOccupiedCells.count; ++i) {
                auto& cell = m_sptlHash[prevOccupiedCells.cells[i]];

                auto it = std::find(cell.begin(), cell.end(), index);
                if (it != cell.end()) cell.erase(it);
            }

            for (uint8_t i = 0; i < newOccupiedCells.count; ++i) {
                m_sptlHash[newOccupiedCells.cells[i]].push_back(index);
            }


        }
    }

    for (const uint8_t& entityIndex : translsToTerminate) m_pEntities[entityIndex]->endMovement();

    resolveCollisions();
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

void WorldScene::resolveCollisions() {

    // Clear last frame's collision data exactly once per entity
    for (auto& e : m_pEntities) e->m_knownCollisions.clear();

    for (const auto& [key, entities] : m_sptlHash) {
        // https://stackoverflow.com/questions/26474995
        for (uint8_t i = 0; i < entities.size(); ++i) {

            for (uint8_t j = i + 1; j < entities.size(); ++j) {

                const uint8_t& a = entities[i];
                const uint8_t& b = entities[j];

                // collision test
                if (intersect(m_pEntities[a]->getAABB(), m_pEntities[b]->getAABB())) {
                    m_pEntities[a]->m_knownCollisions.push_back(b);
                    m_pEntities[b]->m_knownCollisions.push_back(a);

                    if (a == 0) {
                        std::cout << "A is 0\n";
                        // needs more complex logic
                        // interactableEntity = m_pEntities[b];
                        setInteractableEntity(m_pEntities[b].get());
                    }
                    if (b == 0) { // CHECK: why is this the only block that activates, it's meant to be a
                        setInteractableEntity(m_pEntities[a].get());
                    }
                }
            }
        }
    }

    // TODO: quadtree
}



void WorldScene::genSceneSptlHash() {
    for (uint8_t entityIndex = 0; entityIndex < m_pEntities.size(); ++entityIndex) {
        // TODO: The size of geometry might eventually differ between entities, this must be handled! current size is const 1x1
        // insert entity index into all cells that it occupies
        OccupiedCells occupiedCells = getOccupiedCells(m_pEntities[entityIndex]->getAABB());
        for (uint8_t i = 0; i < occupiedCells.count; ++i) m_sptlHash[occupiedCells.cells[i]].push_back(entityIndex);
    }
}
