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
    TYPE_NORMAL    = 0,
    TYPE_FIGHTING  = 1,
    TYPE_FLYING    = 2,
    TYPE_POISON    = 3,
    TYPE_GROUND    = 4,
    TYPE_ROCK      = 5,
    TYPE_BUG       = 6,
    TYPE_GHOST     = 7,
    TYPE_STEEL     = 8,
    TYPE_FIRE      = 9,
    TYPE_WATER     = 10,
    TYPE_GRASS     = 11,
    TYPE_ELECTRIC  = 12,
    TYPE_PSYCHIC   = 13,
    TYPE_ICE       = 14,
    TYPE_DRAGON    = 15,
    TYPE_DARK      = 16,
    TYPE_FAIRY     = 17,
    TYPE_COUNT     = 18 // number of enumerators
};

enum Effectiveness {
    EFFECTIVENESS_IMMUNE,
    EFFECTIVENESS_MILDLY_EFFECTIVE,
    EFFECTIVENESS_NEUTRAL,
    EFFECTIVENESS_SUPER_EFFECTIVE
};

enum MoveCategory {
    MOVE_TYPE_PHYSICAL,
    MOVE_TYPE_SPECIAL,
    MOVE_TYPE_STATUS
};

// game will load all moves from a JSON to a map, where each move has a unique ID for the key
struct Move {
    std::string  name;
    Type         type;
    MoveCategory category;
    int          pp;
    int          power;
    int          accuracy;
};
