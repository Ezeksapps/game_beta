#pragma once

#include "../entity/entity.hpp"
#include "pokemon_common.h"
#include <array>

#include "json.hpp"

using json = nlohmann::json;

class Pokemon : Entity {

public:

    Pokemon(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index);
    ~Pokemon();

private:

    IVs   m_IVs;
    Stats m_stats;

    Nature m_nature;

    // moves limited to 16, because 4 is a stupid limit (this will be configurable in settings)
    std::array<Move, 16> m_moveset;

    std::array<Ability, 2> m_abilities;
    Ability                m_hiddenAbility = ABILITY_NONE;
};
