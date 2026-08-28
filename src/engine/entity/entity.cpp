#include "entity.hpp"
#include "../fileio.hpp"
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

Entity::Entity(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index) {

    m_index = index;
    m_animJsonFilepath = animJsonFilepath;

    // TODO: use JSON from first param to load overworld events/interactions and behaviour

    const char* spriteJson = readJsonAsset(animJsonFilepath.c_str()); // TODO: Fix these functions

    json anims = json::parse(spriteJson)["anims"];

    for (const json& anim : anims) {

        int typeValue = anim["type"];

        Sprite sprite {
            .filepath = anim["filepath"],
            .frameDurations = anim["durations"],
            .frameWidth = anim["frameWidth"],
            .frameHeight = anim["frameHeight"]
        };

        m_spriteMap.insert({(AnimEvent)anim["type"], std::make_shared<Sprite>(sprite)});
    }

    doAnimEvent(ANIM_EVENT_IDLE); // default idle anim
    m_direction = DIRECTION_WEST; // default direction
}


Entity::~Entity() {}

void Entity::update(const float& deltaTime) {

    m_frameTimer += deltaTime;

    // intended duration of current frame of sprite sheet in frames
    float duration = m_pActiveSprite->frameDurations[m_pActiveSprite->frame];

    // if the timer shows that the currently rendered frame has been rendered for a time exceeding the intended duration,
    // reset the timer and advance to next frame
    if (m_frameTimer >= duration) {
        m_frameTimer           = 0.0f;

        m_pActiveSprite->frame = (m_pActiveSprite->frame + 1) % m_pActiveSprite->framesPerRow;
    }
}

void Entity::doAnimEvent(const AnimEvent& event) {
    m_event = event;
    int oldSpriteIndex = 0;
    if (m_pActiveSprite) oldSpriteIndex = m_pActiveSprite->index;
    m_pActiveSprite = m_spriteMap[event];
    // also called to set default anim before setting callback
    if (m_spriteChangeCallback) m_spriteChangeCallback(oldSpriteIndex, m_pActiveSprite);
}

const std::shared_ptr<Sprite>& Entity::getActiveSprite() {
    return m_pActiveSprite;
}

void Entity::setSpriteChangeCallback(std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> callback) {
    m_spriteChangeCallback = callback;
}

void Entity::setMovementCallback(std::function<void(const int& index, vec3 translVec, const float& animFrames)> callback) {
    m_movementCallback = callback;
}

void Entity::setDirection(const Direction& direction) {
    m_direction = direction;
}

bool Entity::isMoving() { return m_isMoving; }

const std::unordered_map<AnimEvent, std::shared_ptr<Sprite>>& Entity::getSpriteMap() {
    return m_spriteMap;
}

// TODO: Must not alter Z-axis, only checkCollision() should update Z
void Entity::move(const Direction& direction, const AnimEvent& mode) {

    if (m_isMoving) return;

    m_isMoving = true;
    m_direction = direction;

    doAnimEvent(mode);

    // find total number of frames accompanying movement animation runs for
    int animFrames = 0;
    for (const int& i : m_pActiveSprite->frameDurations) animFrames += i;

    vec3 translVec;

    switch (direction) {
        case DIRECTION_SOUTH:
            //m_pos += vec3(0.0f, -1.0f, 0.0f);
            translVec = vec3(0.0f, -1.0 / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_SOUTH_EAST:
            //m_pos += vec3(1.0f, -1.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, -1.0f / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_EAST:
            //m_pos += vec3(1.0f, 0.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, 0.0f, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_NORTH_EAST:
            //m_pos += vec3(1.0f, 1.0f, 0.0f);
            translVec = vec3(1.0f / animFrames, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_NORTH:
            //m_pos += vec3(0.0f, 1.0f, 0.0f);
            translVec = vec3(0.0f, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_NORTH_WEST:
            //m_pos += vec3(-1.0f, 1.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, 1.0f / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_WEST:
            //m_pos += vec3(-1.0f, 0.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, 0.0f, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        case DIRECTION_SOUTH_WEST:
            //m_pos += vec3(-1.0f, -1.0f, 0.0f);
            translVec = vec3(-1.0f / animFrames, -1.0f / animFrames, 0.0f /* Z-axis ignored */);
            m_movementCallback(m_index, translVec, animFrames);
            break;
        default:
            break;
    }

    m_isMoving = false;
}
