#pragma once

/* A GameEvent refers to a basic event that is needed for any game that could be using the base engine
 * It is NOT a StoryEvent, these events are not dynamic in their behaviour and follow the logic
 * set in the implementation.
 */

#include <cstdint>

enum GameEvent : uint8_t {
    GAME_EVENT_INTERACT,
    GAME_EVENT_BATTLE_START,
    GAME_EVENT_BATTLE_END,
    GAME_EVENT_ENTER_SCENE
};

