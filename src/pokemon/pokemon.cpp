#include "pokemon.hpp"
#include "../fileio.hpp"
#include "pokemon_common.h"

#include <iostream>

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


}

Pokemon::~Pokemon() {}
