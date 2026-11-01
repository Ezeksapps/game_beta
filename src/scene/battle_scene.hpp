#pragma once

#include "scene.hpp"
#include "../entity/trainer.hpp"

class BattleManager;

class BattleScene : public Scene {
public:

    // player and opponent should be entities already set up by the active WorldScene
    BattleScene(const std::string& sceneDir, std::shared_ptr<Entity> player, std::shared_ptr<Entity> opponent, const std::string& env);
    ~BattleScene();

    void update() override;

    // used only by game event system
    const std::unordered_map<std::string, std::string>& getTexMapping();

    // these will be used by the battle manager so it can add/remove pokemon as needed
    void pushPokemonToScene(std::shared_ptr<Pokemon>& entity);
    void removePokemonFromScene(std::shared_ptr<Pokemon>& entity);

    std::unique_ptr<BattleManager> m_battleManager;

private:

    std::unordered_map<std::string, std::string> m_sceneTexMapping;
    void loadEnvJson(const std::string& jsonFilepath);

};
