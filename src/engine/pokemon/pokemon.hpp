#pragma once

#include "../entity/entity.hpp"
#include <array>

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
    TYPE_NORMAL,
    TYPE_FIGHTING,
    TYPE_FLYING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_ROCK,
    TYPE_BUG,
    TYPE_GHOST,
    TYPE_STEEL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_PSYCHIC,
    TYPE_ICE,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_FAIRY
};

enum MoveCategory {
    MOVE_TYPE_PHYSICAL,
    MOVE_TYPE_SPECIAL,
    MOVE_TYPE_STATUS
};

struct Move {
    std::string  name;
    Type         type;
    MoveCategory category;
    int          pp;
    int          power;
    int          accuracy;
};

class Pokemon : Entity {

public:

    Pokemon();
    ~Pokemon();

private:

    IVs   m_IVs;
    Stats m_stats;

    Nature m_nature;

    // moves limited to 16, because 4 is a stupid limit (this will be configurable in settings)
    std::array<Move, 16> m_moveset;

    std::array<Ability, 2> m_abilities;
    Ability                m_hiddenAbility = ABILITY_NONE;
};
