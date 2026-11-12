#pragma once

/* Common structures and enums that can be included in either C or C++ */

typedef enum Ability { // list kept short for now, only holds abilities of placeholders
    #define X(name) ABILITY_##name,
    #include "abilities.xvals"
    #undef X
    ABILITY_COUNT
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

typedef enum StatusCondition {
    #define X(name) STATUS_CONDITION_##name,
    #include "status_conditions.xvals"
    #undef X
} StatusCondition;

typedef enum Effectiveness {
    EFFECTIVENESS_IMMUNE             = 0,
    EFFECTIVENESS_MILDLY_EFFECTIVE   = 1,
    EFFECTIVENESS_NEUTRAL            = 2,
    EFFECTIVENESS_SUPER_EFFECTIVE    = 3
} Effectiveness;

typedef enum MoveCategory {
    #define X(name) MOVE_TYPE_##name,
    #include "move_types.xvals"
    #undef X
} MoveCategory;
