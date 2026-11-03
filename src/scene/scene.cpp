#include "scene.hpp"

#include <filesystem>
#include <stdexcept>

#include "json.hpp"
#include "../fileio.hpp"

#include "../entity/entity.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

Scene::Scene(const std::string& sceneDir, std::shared_ptr<Entity> player) {

    // do not accept bad paths
    if (!fs::is_directory(sceneDir)) throw std::runtime_error("Scene constructor was provided with a non-directory");

    // do not assume a base class wishes to add the player to its entity vector
    // (BattleScene should only have two Pokemon in that vector, not the player)
}

Scene::~Scene() {}

std::vector<std::shared_ptr<Entity>>& Scene::getEntities() {
    return m_pEntities;
}

const std::string& Scene::getGltfFilepath() {
    return m_glbFilepath;
}

void Scene::setRegisterEntityCallback(std::function<void(const std::shared_ptr<Entity> entity)> callback) {
    m_registerEntityCallback = callback;
}
