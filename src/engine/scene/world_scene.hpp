#pragma once
#include "scene.hpp"


// quadtrees not being worked on currently, sptl hash WIP

class WorldScene : public Scene {

public:

    WorldScene(const std::string& sceneDir, std::shared_ptr<Entity> player);
    ~WorldScene();

    // checks collisions between scene entities & 3D scene meshes
    void resolveCollisions();
    void update() override;

    std::vector<AABB> m_bboxes;

private:

    /* Maps translation vectors to currently moving entities */
    std::unordered_map<int, vec3> m_translsMap;

    void loadSceneJson(const std::string& sceneFilepath);

    void genSceneSptlHash();

    void setupEntityCallbacks(const int& entityIndex);

    // entity ID 8-bit, as 32 < 255
    std::unordered_map<uint32_t, std::vector<uint8_t>> m_sptlHash;
    // bounding boxes for all meshes within this scene
    std::vector<AABB> m_sceneBboxes;
};
