#include "pokemon_utils.hpp"
#include "pokemon.hpp"

#include "json.hpp"
#include "../fileio.hpp"
#include "pokemon_common.hpp"

#include <iostream>

using json = nlohmann::json;

#include <unordered_map>

// cannot be constexpr unless using C++ 26
static const std::unordered_map<std::string, Ability> strToAbilityMap = {
    #define X(name) {#name, Ability::ABILITY_##name},
    #include "abilities.xvals"
    #undef X
};

static const std::unordered_map<std::string, Nature> strToNatureMap = {
    #define X(name) {#name, Nature::NATURE_##name},
    #include "natures.xvals"
    #undef X
};

static const std::unordered_map<std::string, Type> strToTypeMap = {
    #define X(name) {#name, Type::TYPE_##name},
    #include "types.xvals"
    #undef X
};


static const std::unordered_map<std::string, MoveCategory> strToMoveTypeMap = {
    #define X(name) {#name, MoveCategory::MOVE_TYPE_##name},
    #include "move_types.xvals"
    #undef X
};

Ability strToAbility(const std::string& str) {
    return strToAbilityMap.at(str);
}

Nature strToNature(const std::string& str) {
    return strToNatureMap.at(str);
}

Type strToType(const std::string& str) {
    return strToTypeMap.at(str);
}

std::shared_ptr<Pokemon> createPokemonFromDesc(const PokemonDesc& desc, const int& index) {
    /* there is a single JSON database which contains the basic data for all pokemon species
     * which will be located at assets/data/pokemon/, this is separated out so that there is one JSON file
     * per species. The correct file for this Pokemon will be loaded from the species name 'spcname'
     */
    json pokemonJson = json::parse(readJsonAsset(std::string("assets/data/pokemon/" + desc.spcname + ".json").c_str()));

    std::shared_ptr<Pokemon> pokemon = std::make_shared<Pokemon>(desc.spcname, desc.nature, createMovesetFromIDs(desc.moveset),
                                                                 desc.hasHiddenAbility, pokemonJson["sprites"], index);
    return pokemon;
}

void parseMoveJsonCommon(Move& move, const json& moveJson) {
    move.type = strToType(moveJson["type"]);
    move.accuracy = moveJson["accuracy"];
    move.name = moveJson["name"];
    move.pp = moveJson["pp"];
    // used in moves like quick attack
    if (moveJson.contains("priority")) move.priority = moveJson["priority"];
    else move.priority = 0;
}

void parseStatusMoveJson(Move& move, const json& moveJson) {
    // status affecting moves cannot have a power, but can have an accuracy if they affect the stats of the opponent

    /* the effects of a status move are defined in the 'effects' JSON object, which can have these fields:
     * "raiseSelf", "lowerSelf", "raiseOpponent", "lowerOpponent", "inflictSelf", "inflictOpponent"
     * The string values in these arrays are stored in the move object so that the effects on both the player and opponent
     * can be evaluated when executing a move
     */
    json effects = moveJson["effects"];

    // TODO: determine how to best store all possible effects

    parseMoveJsonCommon(move, moveJson);
}

void parseNonStatusMoveJson(Move& move, const json& moveJson) {
    move.power = moveJson["power"];
    parseMoveJsonCommon(move, moveJson);
}

std::array<Move, 4> createMovesetFromIDs(const std::array<std::string, 4>& moveset) {

    std::cout << "Running moveset creator\n";

    std::array<Move, 4> moves;

    for (int i = 0; i < moveset.size(); ++i) {
        json moveJson = json::parse(readJsonAsset(std::string("assets/data/moves/" + moveset[i] + ".json").c_str()));

        Move move;

        std::cout << "Parsing move: " << moveJson["name"] << std::endl;

        switch (strToMoveTypeMap.at(moveJson["category"])) {
            case MOVE_TYPE_PHYSICAL:
                move.category = MOVE_TYPE_PHYSICAL;
                parseNonStatusMoveJson(move, moveJson);
                break;
            case MOVE_TYPE_SPECIAL:
                move.category = MOVE_TYPE_SPECIAL;
                parseNonStatusMoveJson(move, moveJson);
                break;
            case MOVE_TYPE_STATUS:
                parseStatusMoveJson(move, moveJson);
                break;
        }

        moves[i] = move;
    }

    return moves;
}
