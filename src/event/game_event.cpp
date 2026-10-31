#include "game_event.hpp"
#include "../battle/battle_manager.hpp"

// should become part of battle manager init


void triggerEvent(const GameEvent& event) {
    switch (event) {
        case GAME_EVENT_BATTLE_START:
            triggerBattleStart();
            break;
        default:
            break;
    }
}
