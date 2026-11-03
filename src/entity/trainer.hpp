#pragma once

#include "entity.hpp"
#include "../pokemon/pokemon_common.h"
#include "../pokemon/pokemon_utils.hpp"

class Trainer : public Entity {

public:

    Trainer(const std::string& animJsonFilepath, const int& index);
    ~Trainer();

    void loadEntityJSON(const std::string& filepath) override;
    // used by the save read system to init the player's team, since player has no entity JSON
    void loadTeamJSON(const json& teamJson);

    std::array<PokemonDesc, 6> getTeam();

private:

    std::array<PokemonDesc, 6> m_team;

};
