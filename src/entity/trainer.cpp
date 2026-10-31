#include "trainer.hpp"
#include "../fileio.hpp"
#include <json.hpp>

using json = nlohmann::json;

Trainer::Trainer(const std::string& animJsonFilepath, const int& index)
: Entity(animJsonFilepath, index) {


}

Trainer::~Trainer() {}

void Trainer::loadEntityJSON(const std::string& filepath) {
    if (m_index > 0) { // the player has no dialogue or pre-defined behaviour
        const char* entityJson = readJsonAsset(filepath.c_str());

        json dialogStrings = json::parse(entityJson)["dialog"];

        for (json::iterator it = dialogStrings.begin(); it != dialogStrings.end(); ++it) {
            m_dialogStrings.emplace(it.key(), it.value());
        }
    }
}
