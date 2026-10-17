#pragma once
#include "scene.hpp"


// quadtrees not being worked on currently, sptl hash WIP

class WorldScene : Scene {

public:

    WorldScene(const std::string& sceneDir, std::shared_ptr<Entity> player);
    ~WorldScene();

    // checks collisions between scene entities & 3D scene meshes
    void resolveCollisions();

    std::vector<AABB> m_bboxes;

private:

    void loadSceneJson(const std::string& sceneFilepath);

    void genSceneSptlHash();

    // entity ID 8-bit, as 32 < 255
    std::unordered_map<uint32_t, std::vector<uint8_t>> m_sptlHash;
    // bounding boxes for all meshes within this scene
    std::vector<AABB> m_sceneBboxes;
};
