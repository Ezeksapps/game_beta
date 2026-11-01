#pragma once

#include "../entity/trainer.hpp"

class BattleManager {

public:

    BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent);
    ~BattleManager();

private:

    std::shared_ptr<Trainer> m_player;
    std::shared_ptr<Trainer> m_opponent;
};

void triggerBattleStart();
