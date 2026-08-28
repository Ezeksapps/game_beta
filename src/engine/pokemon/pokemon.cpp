#include "pokemon.hpp"
#include "../fileio.hpp"
#include "pokemon_common.h"

Pokemon::Pokemon(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index)
                : Entity(entityJsonFilepath, animJsonFilepath, index) {

    // in addition to the normal properties common to all entities, Pokemon will have other properties
    // defined in their entity JSON that require more specialised handling

    const char* entityJson = readJsonAsset(entityJsonFilepath.c_str());
    json root = json::parse(entityJson);

    json stats = root["stats"];
    m_stats = Stats {
        .attack    = stats["attack"],
        .defense   = stats["defense"],
        .spAttack  = stats["spAttack"],
        .spDefense = stats["spDefense"],
        .speed     = stats["speed"]
    };

    json ivs = root["IVs"];
    m_IVs = IVs {
        .attackIV    = ivs["attack"],
        .defenseIV   = ivs["defense"],
        .spAttackIV  = ivs["spAttack"],
        .spDefenseIV = ivs["spDefense"],
        .speedIV     = ivs["speed"]
    };

    // TODO: Also handle EVs

    for (const std::string& ability : root["abilities"]) {
        // TODO: Map ability strings to corresponding enum val
        // lookup matching Ability enum val and set abilitie(s), same goes for hidden ability
        // This is also an issue for nature
    }
}

Pokemon::~Pokemon() {}
