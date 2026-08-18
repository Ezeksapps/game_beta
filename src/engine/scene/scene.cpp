#include "scene.hpp"

#include <filesystem>
#include <stdexcept>
#include <iostream>

#include "json.hpp"
#include "../fileio.hpp"

#include "../entity/entity.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

Scene::Scene(const std::string& sceneDir) {
    /* TODO: Handle the scene dir name and load the glb (glTF) and JSON in the dir */

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

        std::shared_ptr<Entity> pEntity = std::make_shared<Entity>(entity["json"], entity["sprites"], entityIndex);
        pEntity->m_pos = vec3(entity["posX"], entity["posY"], 0.5f);
        m_pEntities.push_back(pEntity);
        ++entityIndex;
    }
    // TODO: Define scene JSON format
}

