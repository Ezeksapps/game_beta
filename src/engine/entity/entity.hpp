#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "sprite.hpp"
#include "../common.hpp"

using namespace glm;

enum AnimEvent {
    ANIM_EVENT_WALK,
    ANIM_EVENT_RUN,
    ANIM_EVENT_SLEEP,
    ANIM_EVENT_WAKE
};

struct SpriteSheetInfo {
    std::vector<AnimEvent> entityEvents;   // Events that this entity is assigned a spritesheet for
    std::vector<std::string> spriteSheets; // Filepaths for respective spritesheets, in the order in which they were passed to entityEvents
    std::vector<int> eventPages;           // Pages per anim for respective spritesheets, in the order in which they were passed to entityEvents
};



class Entity {

public:

   // Entity(const SpriteSheetInfo& spriteSheetInfo);
    Entity(const std::string& animJsonFilepath);
    ~Entity();

    void doAnimEvent(const AnimEvent& event);

private:

    /* TODO */

    /* Every spritesheet is associated with a certain event/action, which will act as the key in the sprite map
     * An example of a key could be 'walk' or 'run', each of which are separate actions with separate spritesheets
     */
    std::unordered_map<AnimEvent, std::shared_ptr<Sprite>> m_spriteMap;
    std::shared_ptr<Sprite> m_pActiveSprite; // will match whatever the current or last event's corresponding Sprite obj was, set by doAnimEvent()
};
