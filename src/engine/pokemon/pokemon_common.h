#pragma once

#include <string>

/* Common structures and enums */

enum Ability { // list kept short for now, only holds abilities of placeholders

    ABILITY_NONE, // default val for empty slots

    // 133
    ABILITY_RUN_AWAY,
    ABILITY_ADAPTABILITY,
    ABILITY_ANTICIPATION,

    // 37 f
    ABILITY_FLASH_FIRE,
    ABILITY_DROUGHT,

    // 37 i
    ABILITY_SNOW_CLOAK,
    ABILITY_SNOW_WARNING,

    // 403
    ABILITY_INTIMIDATE,
    ABILITY_RIVALRY,
    ABILITY_GUTS,
};

// TODO: Map to affected stats and flavours
enum Nature {
    NATURE_HARDY,
    NATURE_LONELY,
    NATURE_BRAVE,
    NATURE_ADAMANT,
    NATURE_NAUGHTY,
    NATURE_BOLD,
    NATURE_DOCILE,
    NATURE_RELAXED,
    NATURE_IMPISH,
    NATURE_LAX,
    NATURE_TIMID,
    NATURE_HASTY,
    NATURE_SERIOUS,
    NATURE_JOLLY,
    NATURE_NAIVE,
    NATURE_MODEST,
    NATURE_MILD,
    NATURE_QUIET,
    NATURE_BASHFUL,
    NATURE_RASH,
    NATURE_CALM,
    NATURE_GENTLE,
    NATURE_SASSY,
    NATURE_CAREFUL,
    NATURE_QUIRKY
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

// TODO: Need matchup chart
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
