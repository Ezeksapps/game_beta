#pragma once

#include "../entity/entity.hpp"
#include "pokemon_common.h"
#include "pokemon_common.hpp"
#include <array>

#include "json.hpp"

using json = nlohmann::json;

class Pokemon : public Entity {

public:

    // NOTE: Pokemon objects are only constructed by Pokemon descriptors, createPokemonFromDesc handles running this constructor
    Pokemon(const std::string& spcname, const Nature& nature, const std::array<Move, 4>& moveset, const bool& hasHiddenAbility,
            const std::string& animJsonFilepath, const int& index); // TODO: Add IVs and EVs, level and stats
    ~Pokemon();

    void loadEntityJSON(const std::string& filepath) override;

    std::array<Move, 4>& getMoveset();
    const SpcData& getSpcData();

    int m_level;
    Stats m_stats;

    // status conditions will persist outside of battle
    StatusCondition m_statusCondition;

private:

    SpcData m_spcData;
    std::string m_spcname;

    IVs    m_IVs;
    Nature m_nature;

    // move limit will be adjustable later
    std::array<Move, 4> m_moveset;

    std::array<Ability, 2> m_abilities;
    Ability                m_hiddenAbility = ABILITY_NONE;
};
