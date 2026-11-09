#pragma once

#include "pokemon.hpp"
#include "pokemon_common.h"

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

/* Pre-loader functions for JSON data needed for handling Pokemon-type Entity instances */

Ability strToAbility(const std::string& str);
Nature strToNature(const std::string& str);
Type strToType(const std::string& str);
// may also need reverse funcs (for saves)

std::shared_ptr<Pokemon> createPokemonFromDesc(const PokemonDesc& desc, const int& index);
std::array<Move, 4> createMovesetFromIDs(const std::array<std::string, 4>& moveset);
Effectiveness getMoveEffectiveness(const Type& moveType, const std::array<Type, 2>& targetTypes);
