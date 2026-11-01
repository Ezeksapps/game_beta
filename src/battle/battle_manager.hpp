#pragma once

#include "../entity/trainer.hpp"
#include "../scene/battle_scene.hpp"

class BattleManager {

public:

    // the non-owning pointer to battle scene is necessary to switch out the active Pokemon objects
    BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent, BattleScene* scene);
    ~BattleManager();

private:

    std::shared_ptr<Trainer> m_player;
    std::shared_ptr<Trainer> m_opponent;
    BattleScene* m_scene;
};

void triggerBattleStart();
