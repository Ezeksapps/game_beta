#pragma once

#include "../entity/entity.hpp"
#include "glm/fwd.hpp"
#include <cstdint>

class Scene {

public:

    Scene(const std::string& sceneDir, std::shared_ptr<Entity> player);
    ~Scene();

    std::vector<std::shared_ptr<Entity>>& getEntities();
    const std::string& getGltfFilepath();
    virtual void update() = 0;

protected:

    std::vector<std::shared_ptr<Entity>> m_pEntities;
    std::string m_glbFilepath;

private:

};
