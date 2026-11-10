#include "pokemon.hpp"
#include "../fileio.hpp"
#include "pokemon_common.h"
#include "pokemon_common.hpp"
#include "pokemon_utils.hpp"

#include <iostream>

Pokemon::Pokemon(const std::string& spcname, const Nature& nature, const std::array<Move, 4>& moveset, const bool& hasHiddenAbility,
                const std::string& animJsonFilepath, const int& index) : Entity(animJsonFilepath, index) {

    m_spcname = spcname;
    m_nature = nature;
    m_moveset = moveset;

    std::string spcDataFilepath = "assets/data/pokemon/" + spcname + ".json";

    std::cout << "Attempting to parse species data\n";

    json spcDataJson = json::parse(readJsonAsset(spcDataFilepath.c_str()));

    std::cout << "parsed\n";

    bool isSingleType = spcDataJson["types"].is_array();
    bool hasSingleAbility = spcDataJson["abilities"].is_array();

    // [json.exception.type_error.305] cannot use operator[] with a numeric argument with string
    // ? operator being used for array indexing?

    std::array<Type, 2> types;
    if (isSingleType) types = { strToType(spcDataJson["types"].get<std::string>()), TYPE_COUNT };
    else types = { strToType(spcDataJson["types"][0]), strToType(spcDataJson["types"][1]) };

    std::array<Ability, 2> abilities;
    if (hasSingleAbility) abilities = { strToAbility(spcDataJson["abilities"].get<std::string>()), ABILITY_COUNT };
    else abilities = { strToAbility(spcDataJson["abilities"][0]), strToAbility(spcDataJson["abilities"][1]) };

    m_spcData = SpcData {
        .name = spcDataJson["name"],
        .isSingleType = isSingleType,
        .hasSingleAbility = hasSingleAbility,
        .types = types,
        .baseStats = Stats {0}, // TODO: Load and use these (probably used for generating wild pkmn)
        .abilities = abilities,
        .hiddenAbility = strToAbility(spcDataJson["hiddenAbility"])
    };
}

void Pokemon::loadEntityJSON(const std::string& filepath) {

    const char* entityJson = readJsonAsset(filepath.c_str());

    if (m_index > 0) { // the player has no dialogue or pre-defined behaviour

        json dialogStrings = json::parse(entityJson)["dialog"];

        for (json::iterator it = dialogStrings.begin(); it != dialogStrings.end(); ++it) {
            m_dialogStrings.emplace(it.key(), it.value());
        }
    }
}

Pokemon::~Pokemon() {}

std::array<Move, 4>& Pokemon::getMoveset() { return m_moveset; }

const SpcData& Pokemon::getSpcData() { return m_spcData; }
