#pragma once

#include <string>

/* Common structures and enums */

enum Ability { // list kept short for now, only holds abilities of placeholders
#define X(name) ABILITY_##name,
#include "abilities.xvals"
#undef X
};

// TODO: Map to affected stats and flavours
enum Nature {
#define X(name) NATURE_##name,
#include "natures.xvals"
#undef X
};

struct Stats {
    int attack;
    int defense;
    int spAttack;
    int spDefense;
    int speed;
    int hp;
};

struct IVs {
    int attackIV;
    int defenseIV;
    int spAttackIV;
    int spDefenseIV;
    int speedIV;
    int hpIV;
};

enum Type {
    #define X(name) TYPE_##name,
    #include "types.xvals"
    #undef X
    TYPE_COUNT // number of enumerators
};

enum Effectiveness {
    EFFECTIVENESS_IMMUNE,
    EFFECTIVENESS_MILDLY_EFFECTIVE,
    EFFECTIVENESS_NEUTRAL,
    EFFECTIVENESS_SUPER_EFFECTIVE
};

enum MoveCategory {
    #define X(name) MOVE_TYPE_##name,
    #include "move_types.xvals"
    #undef X
};

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
