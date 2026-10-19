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

    m_pEntities.push_back(player); // add player to entities first
}

Scene::~Scene() {}

std::vector<std::shared_ptr<Entity>>& Scene::getEntities() {
    return m_pEntities;
}

const std::string& Scene::getGltfFilepath() {
    return m_glbFilepath;
}
