#pragma once

#include "entity.hpp"
#include "../pokemon/pokemon_common.h"
#include <array>


// descriptor for a Pokemon object, used because creating an actual Pokemon object will immediately
// cause the renderer to display it, and the game should only display the two Pokemon actively battling,
// not the whole party
struct PokemonDesc {
    std::string spcname;
    std::string name;
    Nature nature;
    std::array<std::string, 4> moveset; // the Move structures are loaded as needed, just store their str IDs
    int level;
    int exp;
    bool hasHiddenAbility;
};

class Trainer : public Entity {

public:

    Trainer(const std::string& animJsonFilepath, const int& index);
    ~Trainer();

    void loadEntityJSON(const std::string& filepath) override;

    std::array<PokemonDesc, 6> getTeam();

private:

    std::array<PokemonDesc, 6> m_team;

};
