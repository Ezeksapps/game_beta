#pragma once

#include "../entity/trainer.hpp"
#include "../scene/battle_scene.hpp"

class BattleManager {

public:

    // the non-owning pointer to battle scene is necessary to switch out the active Pokemon objects
    BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent, BattleScene* scene);
    ~BattleManager();

    // prepare the stage with both players' pokemon (not done on init, as this requires the scene to be set in the renderer to work)
    void prepareStage(std::shared_ptr<Pokemon> playerPkmn, std::shared_ptr<Pokemon> opponentPkmn);

private:

    std::shared_ptr<Trainer> m_player;
    std::shared_ptr<Trainer> m_opponent;
    BattleScene* m_scene;
};

void triggerBattleStart();
