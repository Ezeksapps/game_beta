#include "pokemon.hpp"
#include "../fileio.hpp"
#include "pokemon_common.h"

Pokemon::Pokemon(const std::string& spcname, const std::string& animJsonFilepath, const int& index)
                : Entity(animJsonFilepath, index) { m_spcname = spcname; }

void Pokemon::loadEntityJSON(const std::string& filepath) {

    const char* entityJson = readJsonAsset(filepath.c_str());

    if (m_index > 0) { // the player has no dialogue or pre-defined behaviour

        json dialogStrings = json::parse(entityJson)["dialog"];

        for (json::iterator it = dialogStrings.begin(); it != dialogStrings.end(); ++it) {
            m_dialogStrings.emplace(it.key(), it.value());
        }
    }


    // in addition to the normal properties common to all entities, Pokemon will have other properties
    // defined in their entity JSON that require more specialised handling

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
