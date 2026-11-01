#include "pokemon_utils.hpp"

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
