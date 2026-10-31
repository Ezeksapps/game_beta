#pragma once

#include "../pokemon/pokemon_common.h"
#include "../entity/trainer.hpp"
#include <array>

// descriptor for a Pokemon object, used because creating an actual Pokemon object will immediately
// cause the renderer to display it, and the game should only display the two Pokemon actively battling,
// not the whole party
struct PokemonDesc {
    std::string spcname;
    std::string name;
    Nature nature;
    std::array<Move, 16> moveset;
    int level;
    int exp;
    bool hasHiddenAbility;
};

class BattleManager {

public:

    BattleManager(std::shared_ptr<Trainer> player, std::shared_ptr<Trainer> opponent);
    ~BattleManager();

private:

    std::shared_ptr<Trainer> m_player;
    std::shared_ptr<Trainer> m_opponent;
};

void triggerBattleStart();
