#pragma once

#include "scene.hpp"

class BattleScene : Scene {
public:

    // player and opponent should be entities already set up by the active WorldScene
    BattleScene(const std::string& sceneDir, std::shared_ptr<Entity> player, std::shared_ptr<Entity> opponent);
    ~BattleScene();

private:


};
