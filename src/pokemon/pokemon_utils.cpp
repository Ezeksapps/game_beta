#include "pokemon_utils.hpp"
#include "pokemon.hpp"

#include "json.hpp"
#include "../fileio.hpp"

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

Ability strToAbility(const std::string& str) {
    return strToAbilityMap.at(str);
}

Nature strToNature(const std::string& str) {
    return strToNatureMap.at(str);
}

std::shared_ptr<Pokemon> createPokemonFromDesc(const PokemonDesc& desc, const int& index) {
    /* there is a single JSON database which contains the basic data for all pokemon species
     * which will be located at assets/data/pokemon/, this is separated out so that there is one JSON file
     * per species. The correct file for this Pokemon will be loaded from the species name 'spcname'
     */
    json pokemonJson = json::parse(readJsonAsset(std::string("assets/data/pokemon/" + desc.spcname + ".json").c_str()));

    std::shared_ptr<Pokemon> pokemon = std::make_shared<Pokemon>(desc.spcname, pokemonJson["sprites"], index);
    return pokemon;
}
