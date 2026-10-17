#pragma once

#include "../entity/entity.hpp"
#include "glm/fwd.hpp"
#include <cstdint>

class Scene {

friend class Renderer; // TODO remove

public:

    Scene(const std::string& sceneDir, std::shared_ptr<Entity> player);
    ~Scene();

    std::vector<std::shared_ptr<Entity>>& getEntities();

    std::vector<std::shared_ptr<Entity>> m_pEntities; // TEMP

protected:

    std::string m_glbFilepath;

private:


};
