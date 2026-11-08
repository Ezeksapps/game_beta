#pragma once

/* Common structures and enums that can be included in either C or C++ */

typedef enum Ability { // list kept short for now, only holds abilities of placeholders
    #define X(name) ABILITY_##name,
    #include "abilities.xvals"
    #undef X
} Ability;

// TODO: Map to affected stats and flavours
typedef enum Nature {
    #define X(name) NATURE_##name,
    #include "natures.xvals"
    #undef X
} Nature;

typedef struct Stats {
    int attack;
    int defense;
    int spAttack;
    int spDefense;
    int speed;
    int hp;
} Stats;

typedef struct IVs {
    int attackIV;
    int defenseIV;
    int spAttackIV;
    int spDefenseIV;
    int speedIV;
    int hpIV;
} IVs;

typedef enum Type {
    #define X(name) TYPE_##name,
    #include "types.xvals"
    #undef X
    TYPE_COUNT // number of enumerators
} Type;

typedef enum Effectiveness {
    EFFECTIVENESS_IMMUNE,
    EFFECTIVENESS_MILDLY_EFFECTIVE,
    EFFECTIVENESS_NEUTRAL,
    EFFECTIVENESS_SUPER_EFFECTIVE
} Effectiveness;

typedef enum MoveCategory {
    #define X(name) MOVE_TYPE_##name,
    #include "move_types.xvals"
    #undef X
} MoveCategory;

/* --- C Data packets --- */

// A version of the C++-side Move structure for the UI system
typedef struct MoveC {
    const char*  name;
    Type         type;
    MoveCategory category;
    int          pp;
    int          power;
    int          accuracy;
    int          priority;
} MoveC;

// user data for move selection screen
typedef struct MovesetDataC {
    MoveC* moveset;
    void* battleManager;
} MovesetDataC;
