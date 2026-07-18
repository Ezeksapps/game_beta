#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "sprite.hpp"
#include "../common.hpp"

using namespace glm;

enum AnimEvent : uint8_t {
    ANIM_EVENT_WALK,
    ANIM_EVENT_RUN,
    ANIM_EVENT_SLEEP,
    ANIM_EVENT_WAKE
};

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

    Entity(const std::string& animJsonFilepath);
    ~Entity();

    void doAnimEvent(const AnimEvent& event);
    void setDirection(const Direction& direction);

private:

    /* TODO */

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets
     */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    std::shared_ptr<Sprite> m_pActiveSprite; // will match whatever the current or last event's corresponding Sprite obj was, set by doAnimEvent()
    Direction m_direction;
};
