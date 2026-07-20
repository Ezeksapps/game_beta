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

    void setSpriteChangeCallback(std::function<void(std::shared_ptr<Entity> entity)> callback);

    void doAnimEvent(const AnimEvent& event);
    const std::shared_ptr<Sprite>& getActiveSprite();
    void setDirection(const Direction& direction);

    // update frame timing data based on the delta time of the renderer
    void update(const float& deltaTime);

    Direction m_direction;

private:

    /* callback for sprite sheet animation changes */
    std::function<void(std::shared_ptr<Entity> entity)> m_spriteChangeCallback;

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    // will match whatever the current or last event's corresponding Sprite obj was, set by doAnimEvent()
    std::shared_ptr<Sprite> m_pActiveSprite;

    float m_frameTimer;
};
