#include "trainer.hpp"
#include "../fileio.hpp"
#include <json.hpp>

#include "../pokemon/pokemon_utils.hpp"

using json = nlohmann::json;

Trainer::Trainer(const std::string& animJsonFilepath, const int& index)
: Entity(animJsonFilepath, index) {


}

Trainer::~Trainer() {}

void Trainer::loadEntityJSON(const std::string& filepath) {
    const char* entityJson = readJsonAsset(filepath.c_str());

    if (m_index > 0) { // the player has no dialogue or pre-defined behaviour

        json dialogStrings = json::parse(entityJson)["dialog"];

        for (json::iterator it = dialogStrings.begin(); it != dialogStrings.end(); ++it) {
            m_dialogStrings.emplace(it.key(), it.value());
        }
    }

    json team = json::parse(entityJson)["team"];

    // This team is the trainer's team of Pokemon, these will be represented as Pokemon descriptors

    int counter = 0;
    for (const json& pokemon : team) {
        PokemonDesc desc = {
            .spcname          = pokemon["spcname"],
            .name             = pokemon["name"],
            .nature           = strToNature(pokemon["nature"]),
            .moveset          = pokemon["moveset"],
            .level            = pokemon["level"],
            .exp              = pokemon["exp"],
            .hasHiddenAbility = pokemon["hasHiddenAbility"]
        };
        m_team[counter] = desc;
        ++counter;
    }
}

std::array<PokemonDesc, 6> Trainer::getTeam() { return m_team; }
