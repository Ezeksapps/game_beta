#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "sprite.hpp"
#include "../common.hpp"

using namespace glm;

enum AnimEvent : uint8_t {
    ANIM_EVENT_IDLE  = 0,
    ANIM_EVENT_WALK  = 1,
    ANIM_EVENT_COUNT = 2      // Number of enumerators in AnimEvent, used to create properly-sized texture caches in renderer
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

    Entity(const std::string& entityJsonFilepath, const std::string& animJsonFilepath, const int& index);
    ~Entity();

    void setSpriteChangeCallback(std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> callback);
    void setMovementCallback(std::function<void(const int& index, vec3 translVec, const float& animFrames)> callback);

    void doAnimEvent(const AnimEvent& event);
    const std::shared_ptr<Sprite>& getActiveSprite();
    void setDirection(const Direction& direction);

    // update frame timing data based on the delta time of the renderer
    void update(const float& deltaTime);

    // used by renderer, returns a read-only reference to the sprite map
    const std::unordered_map<AnimEvent, std::shared_ptr<Sprite>>& getSpriteMap();

    // move entity by one square in one direction, with a specified mode of transport (walking/running)
    void move(const Direction& direction, const AnimEvent& mode);

    bool isMoving(); // getter, checks movement status of Entity

    // CHECK: Should be private members?
    Direction m_direction;
    AnimEvent m_event;        // current event
    vec3 m_pos;               // Position (before accounting for world-view-model matrix)
    int m_index;              // index/number of entity in Scene

    // filepath for the JSON defining this entity's sprite sheets, used by the renderer to lookup the cached textures associated with this Entity
    std::string m_animJsonFilepath;

protected:

    /* callback for sprite sheet animation changes */
    std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> m_spriteChangeCallback;
    /* Callback for movement */
    std::function<void(const int& index, vec3 translVec, const float& animFrames)> m_movementCallback;

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    // will match whatever the current or last event's corresponding Sprite obj was set by doAnimEvent()
    std::shared_ptr<Sprite> m_pActiveSprite;

    float m_frameTimer;
    bool m_isMoving; // controls movement, move() can only be called if there is no existing movement
};
