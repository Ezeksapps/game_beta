#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "sprite.hpp"
#include "../common.hpp"

using namespace glm;

enum AnimEvent : uint8_t { // TODO: Add Idle animation as well
    ANIM_EVENT_IDLE,
    ANIM_EVENT_WALK,
    ANIM_EVENT_RUN,
    ANIM_EVENT_SLEEP,
    ANIM_EVENT_WAKE
};

// TODO: Some sprites have only one anim row, as they are direction-independent, accomodate these as well
enum Direction : uint8_t {
    DIRECTION_SOUTH       = 0,
    DIRECTION_SOUTH_EAST  = 1,
    DIRECTION_EAST        = 2,
    DIRECTION_NORTH_EAST  = 3,
    DIRECTION_NORTH       = 4,
    DIRECTION_NORTH_WEST  = 5,
    DIRECTION_WEST        = 6,
    DIRECTION_SOUTH_WEST  = 7
};

class Entity {

public:

    Entity(const std::string& animJsonFilepath, const int& index);
    ~Entity();

    void setSpriteChangeCallback(std::function<void(std::shared_ptr<Sprite> newSprite)> callback);
    void setMovementCallback(std::function<void(const int& index, vec3 translVec, const float& animFrames)> callback);

    void doAnimEvent(const AnimEvent& event);
    const std::shared_ptr<Sprite>& getActiveSprite();
    void setDirection(const Direction& direction);

    // update frame timing data based on the delta time of the renderer
    void update(const float& deltaTime);

    // move entity by one square in one direction, with a specified mode of transport (walking/running)
    void move(const Direction& direction, const AnimEvent& mode);

    // CHECK: Should be private members?
    Direction m_direction;
    vec3 m_pos;               // Position (before accounting for world-view-model matrix)
    int m_index;              // index/number of entity in Scene

private:

    /* callback for sprite sheet animation changes */
    std::function<void(std::shared_ptr<Sprite> newSprite)> m_spriteChangeCallback;
    /* Callback for movement */
    std::function<void(const int& index, vec3 translVec, const float& animFrames)> m_movementCallback;

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    // will match whatever the current or last event's corresponding Sprite obj was set by doAnimEvent()
    std::shared_ptr<Sprite> m_pActiveSprite;

    float m_frameTimer;
};
