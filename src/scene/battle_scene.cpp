#include "battle_scene.hpp"

#include <filesystem>
#include <stdexcept>

#include "json.hpp"
#include "../fileio.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

BattleScene::BattleScene(const std::string& sceneDir, std::shared_ptr<Entity> player, std::shared_ptr<Entity> opponent, const std::string& env)
                         : Scene::Scene(sceneDir, player) {

    fs::path dir = sceneDir;

    // find the two needed files (The scene JSON and glTF and validate directory)
    fs::path jsonFilepath, gltfFilepath;
    int gltfCounter = 0;
    bool envJsonPresent = false; // check that the JSON needed for the environment exists

    for (const fs::directory_entry& file : fs::recursive_directory_iterator(dir)) {

        if (gltfCounter > 1) throw std::runtime_error("Scene directory contains more than one glTF file");
        if (!fs::is_regular_file(file)) continue; // ignore any non-standard files

        // the environment string passed must match the name of the environment JSON, this check is case-sensitive
        if (file.path().extension() == ".json" && file.path().stem() == env) envJsonPresent = true;

        if (file.path().extension() == ".glb") {
            ++gltfCounter;
            gltfFilepath = file.path();
        }
    }

    m_glbFilepath = gltfFilepath;

    loadEnvJson(jsonFilepath);
}

BattleScene::~BattleScene() {}

void BattleScene::loadEnvJson(const std::string& jsonFilepath) {
    // NOTE: This scene JSON doesn't contain any pre-set entities, the only entities that should be in the scene are the player and the opponent

    /* Battle scene environment JSON filepath format:
     * "backgroundTex": <filepath>,
     * "foregroundTex": <filepath>,
     * "playerStageTex": <filepath>,
     * "opponentStageTex" <filepath>
     */

    json envJson = json::parse(readJsonAsset(jsonFilepath.c_str()));

    m_sceneTexMapping.emplace("background", envJson["backgroundTex"].get<std::string>());
    m_sceneTexMapping.emplace("foreground", envJson["foregroundTex"].get<std::string>());
    m_sceneTexMapping.emplace("playerStage", envJson["playerStageTex"].get<std::string>());
    m_sceneTexMapping.emplace("opponentStage", envJson["opponentStageTex"].get<std::string>());

    // Scene object will not do anything more with this data, tex mapping will only be used by game event system
    // if GAME_EVENT_BATTLE_START is triggered

}

const std::unordered_map<std::string, std::string>& BattleScene::getTexMapping() {
    return m_sceneTexMapping;
}

void BattleScene::update() {}
