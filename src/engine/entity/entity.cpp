#include "entity.hpp"
#include "../fileio.hpp"

#include <json.hpp>

using json = nlohmann::json;

Entity::Entity(const std::string& animJsonFilepath) {

    const char* spriteJson = readJsonAsset(animJsonFilepath.c_str()); // TODO: Fix these functions
    json anims = json::parse(spriteJson)["anims"];

    for (const json& anim : anims) {

        Sprite sprite {
            .filepath = anim["filepath"],
            .frameDurations = anim["durations"],
        };

        m_spriteMap.insert({(AnimEvent)anim["type"], std::make_shared<Sprite>(sprite)});
    }

}


Entity::~Entity() {}

void Entity::doAnimEvent(const AnimEvent& event) {
    m_pActiveSprite = m_spriteMap[event];
}

const std::shared_ptr<Sprite>& Entity::getActiveSprite() {
    return m_pActiveSprite;
}

void Entity::setSpriteChangeCallback(std::function<void(std::shared_ptr<Entity> entity)> callback) {
    m_spriteChangeCallback = callback;
}

void Entity::setDirection(const Direction& direction) {
    m_direction = direction;
}

