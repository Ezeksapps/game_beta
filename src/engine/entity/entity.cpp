#include "entity.hpp"
#include "../fileio.hpp"
#include <iostream>

#include <json.hpp>

using json = nlohmann::json;

Entity::Entity(const std::string& animJsonFilepath) {

    const char* spriteJson = readJsonAsset(animJsonFilepath.c_str()); // TODO: Fix these functions

    json anims = json::parse(spriteJson)["anims"];

    for (const json& anim : anims) {

        Sprite sprite {
            .filepath = anim["filepath"],
            .frameDurations = anim["durations"]
        };

        m_spriteMap.insert({(AnimEvent)anim["type"], std::make_shared<Sprite>(sprite)});
    }

    doAnimEvent(ANIM_EVENT_WALK); // PLACEHOLDER, REMOVE FOLLOWING TEST
    m_direction = DIRECTION_WEST;

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
        m_pActiveSprite->frame = (m_pActiveSprite->frame + 1) % m_pActiveSprite->framesPerRow;
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
/*
enum Direction : uint8_t {
    DIRECTION_SOUTH       = 0,
    DIRECTION_SOUTH_EAST  = 1,
    DIRECTION_EAST        = 2,
    DIRECTION_NORTH_EAST  = 3,
    DIRECTION_NORTH       = 4,
    DIRECTION_NORTH_WEST  = 5,
    DIRECTION_WEST        = 6,
    DIRECTION_SOUTH_WEST  = 7
};*/

// TODO: Must not alter Z-axis, checkCollision() must only update Z
void Entity::move(const Direction& direction, const AnimEvent& mode) {
    doAnimEvent(mode);
    switch (direction) {
        case DIRECTION_SOUTH:
            m_pos += vec3(0.0f, -1.0f, 0.0f);
            break;
        case DIRECTION_SOUTH_EAST:
            m_pos += vec3(1.0f, -1.0f, 0.0f);
            break;
        case DIRECTION_EAST:
            m_pos += vec3(1.0f, 0.0f, 0.0f);
            break;
        case DIRECTION_NORTH_EAST:
            m_pos += vec3(1.0f, 1.0f, 0.0f);
            break;
        case DIRECTION_NORTH:
            m_pos += vec3(0.0f, 1.0f, 0.0f);
            break;
        case DIRECTION_NORTH_WEST:
            m_pos += vec3(-1.0f, 1.0f, 0.0f);
            break;
        case DIRECTION_WEST:
            m_pos += vec3(-1.0f, 0.0f, 0.0f);
            break;
        case DIRECTION_SOUTH_WEST:
            m_pos += vec3(-1.0f, -1.0f, 0.0f);
            break;
        default:
            break;
    }
}
