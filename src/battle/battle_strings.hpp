#pragma once

#include <string>
#include <unordered_map>

enum BattleStr {
    STR_AWAIT_ACTION,
    STR_PLAYER_USE_MOVE,
    STR_OPPONENT_USE_MOVE,
    STR_PLAYER_USE_ITEM,
    STR_OPPONENT_USE_ITEM,
    STR_PLAYER_SWITCH_POKEMON,
    STR_OPPONENT_SWITCH_POKEMON,
    STR_DAMAGE_IMMUNE_PLAYER,
    STR_DAMAGE_IMMUNE_OPPONENT,
    STR_DAMAGE_MILDLY_EFFECTIVE,
    STR_DAMAGE_SUPER_EFFECTIVE
    // add status effect strings, HP leech strings, etc.
};

static const std::unordered_map<BattleStr, std::string> battleStrings = {
    {STR_AWAIT_ACTION, "What will [player_pokemon_name] do?"},
    {STR_PLAYER_USE_MOVE, "[player_pokemon_name] used %"},
    {STR_OPPONENT_USE_MOVE, "[opponent_pokemon_name] used %"},
    {STR_DAMAGE_IMMUNE_PLAYER, "It doesn't affect [player_pokemon_name]..."},
    {STR_DAMAGE_IMMUNE_OPPONENT, "It doesn't affect [opponent_pokemon_name]..."},
    {STR_DAMAGE_MILDLY_EFFECTIVE, "It's not very effective..."},
    {STR_DAMAGE_SUPER_EFFECTIVE, "It's super effective!"}
    // TODO: add missing strings and figure out how to deal with move names
};
