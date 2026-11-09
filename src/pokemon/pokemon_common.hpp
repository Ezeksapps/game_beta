#pragma once

#include <string>
#include <array>
#include "pokemon_common.h"

/* Common structures and enums, specific to C++ */

// move data is stored as JSON assets and is lazy-loaded
struct Move {
    std::string  name;
    Type         type;
    MoveCategory category;
    int          pp;
    int          power;
    int          accuracy;
    int          priority;
};

struct SpcData {
    std::string name;
    // TODO: add experience gain class
    bool isSingleType;
    bool hasSingleAbility;
    std::array<Type, 2> types;
    Stats baseStats;
    std::array<Ability, 2> abilities;
    Ability hiddenAbility;
};
