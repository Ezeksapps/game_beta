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

void Entity::update(const float& deltaTime) {

    m_frameTimer += deltaTime;

    // intended duration of current frame in ms
    float duration = m_pActiveSprite->frameDurations[m_pActiveSprite->frame];

    // if the timer shows that the currently rendered frame has been rendered for a time exceeding the intended duration,
    // reset the timer and advance to next frame
    if (m_frameTimer >= duration) {
        m_frameTimer           = 0.0f;
        m_pActiveSprite->frame = (m_currentFrame + 1) % m_pActiveSprite->framesPerRow;
    }
}

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

