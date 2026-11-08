#pragma once

#include <string>
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
