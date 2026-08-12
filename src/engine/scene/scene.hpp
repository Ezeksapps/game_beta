#pragma once

/* Every 'map' is both the GLB with the visual scene and a JSON controlling the Entities and other metadata contained in a full scene
 * This class wraps all of this data. The Engine can only hold one instance of a loaded scene
 */

#include "../entity/entity.hpp"

class Scene {

friend class Renderer; // TODO remove
friend class Engine; // TODO remove

public:

    Scene(const std::string& sceneDir);
    ~Scene();

    std::vector<std::shared_ptr<Entity>>& getEntities();
        std::vector<std::shared_ptr<Entity>> m_pEntities; // TEMP

private:


    std::string m_glbFilepath;

};
