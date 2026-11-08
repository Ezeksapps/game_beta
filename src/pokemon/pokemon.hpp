#pragma once

#include "../entity/entity.hpp"
#include "pokemon_common.hpp"
#include <array>

#include "json.hpp"

using json = nlohmann::json;

class Pokemon : public Entity {

public:

    // NOTE: Pokemon objects are only constructed by Pokemon descriptors, createPokemonFromDesc handles running this constructor
    Pokemon(const std::string& spcname, const Nature& nature, const std::array<Move, 4>& moveset, const bool& hasHiddenAbility,
            const std::string& animJsonFilepath, const int& index); // TODO: Add IVs and EVs
    ~Pokemon();

    void loadEntityJSON(const std::string& filepath) override;

    std::array<Move, 4>& getMoveset();

private:

    std::string m_spcname;

    IVs   m_IVs;
    Stats m_stats;

    Nature m_nature;

    // move limit will be adjustable later
    std::array<Move, 4> m_moveset;

    std::array<Ability, 2> m_abilities;
    Ability                m_hiddenAbility = ABILITY_NONE;
};
