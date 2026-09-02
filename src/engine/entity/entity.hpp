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

    /* Callback Setters */

    void setSpriteChangeCallback(std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> callback);
    void setStartMovementCallback(std::function<void(const int& index, vec3 translVec)> callback);
    void setChangeMovementDirectionCallback(std::function<void(const int& index, vec3 translVec)> callback);
    void setEndMovementCallback(std::function<void(const int& index)> callback);

    /* General Setters */

    void doAnimEvent(const AnimEvent& event);

    /* Sprite-related funcs */

    // update frame timing data based on the delta time of the renderer
    void update(const float& deltaTime);

    // used by renderer, returns a read-only reference to the sprite map
    const std::unordered_map<AnimEvent, std::shared_ptr<Sprite>>& getSpriteMap();
    // used by renderer, returns a read-only reference to the active sprite
    const std::shared_ptr<Sprite>& getActiveSprite();

    int getIndex();
    std::string getCacheKey(); // the key being the filepath to the JSON outlining the spritesheets used by this entity

    // move in a specified direction indefinitely with a specified mode of transport (walking/running)
    void move(const Direction& direction, const AnimEvent& mode);
    // change direction of already active movement (required to update the translVec to reflect new direction, updating
    // m_direction directly in this case would keep moving the entity in the previous direction, and only update the spritesheet animation)
    void changeMovementDirection(const Direction& direction);
    // end current movement and return to ANIM_EVENT_IDLE
    void endMovement();

    bool isMoving(); // getter, checks movement status of Entity

    // TODO: Move read-only members to private and use getters
    // read-only: index, pos (set initially by Scene, maybe make Entity friend class of Scene?), event (shouldnt be changed without ops performed by doAnimEvent)
    // r/w: direction
    Direction m_direction;
    AnimEvent m_event;        // current event
    vec3 m_pos;               // Position (before accounting for world-view-model matrix)
    int m_index;              // index/number of entity in Scene

protected:
private:

    // filepath for the JSON defining this entity's sprite sheets, used by the renderer to lookup the cached textures associated with this Entity
    std::string m_animJsonFilepath;

    /* Callbacks */
    std::function<void(const int& oldSpriteIndex, std::shared_ptr<Sprite> newSprite)> m_spriteChangeCallback;
    std::function<void(const int& index, vec3 translVec)> m_startMovementCallback;
    std::function<void(const int& index, vec3 translVec)> m_changeMovementDirectionCallback;
    std::function<void(const int& index)> m_endMovementCallback;

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    // will match whatever the current or last event's corresponding Sprite obj was set by doAnimEvent()
    std::shared_ptr<Sprite> m_pActiveSprite;

    float m_frameTimer;
    bool m_isMoving; // controls movement, move() can only be called if there is no existing movement

    /* total number of frames the currently active movement sprite sheet animation lasts for, used to re-calculate
     * translVec if changeMovementDirection() is called, as the translVec calculation requires this value.
     * This member is not used for any sprite sheet animation that does not correspond to movement
     */
    int m_animFrames;
};
