#include "scene.hpp"
#include <iostream>

Scene::Scene(const std::string& sceneDir) {
    /* TODO: Handle the scene dir name and load the glb (glTF) and JSON in the dir */

    // PLACEHOLDER CODE: ONLY ACTS AS A TEST FOR THE ENTITY AND SPRITSHEET SYSTEM
    m_pEntities.emplace_back(std::make_shared<Entity>("eevee.json", 0));
    m_pEntities[0]->m_pos = vec3(0.0f, 0.0f, 0.5f);
    m_glbFilepath = "assets/test.glb";
}

Scene::~Scene() {}

//std::vector<std::shared_ptr<Entity>> Scene::getEntities() {
//    return m_pEntities;
//}
