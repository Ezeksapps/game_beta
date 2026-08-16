#include "game.hpp"
#include "engine/common.hpp"
#include <iostream>

std::vector<std::shared_ptr<Entity>>* entities = nullptr;

void handleInput(GameCmd* input) {
    switch(*input) {
        case ENTITY_MOVE_FORWARD:
            (*entities)[0]->move(DIRECTION_NORTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_LEFT:
            (*entities)[0]->move(DIRECTION_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_RIGHT:
            (*entities)[0]->move(DIRECTION_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_BACKWARD:
            (*entities)[0]->move(DIRECTION_SOUTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_RUN:
            break;
        case UI_PROGRESS:
            break;
        case UI_ESCAPE:
            break;
        case UI_OPEN:
            break;
        case SHORTCUT_BAG_OPEN:
            break;
        case SHORTCUT_MAP_OPEN:
            break;
        case SHORTCUT_PKMN_OPEN:
            break;
        case ACTIVATE_DEBUG_OVERLAY:
            break;
        case GAME_CAPTURE_SCREEN:
            break;
    }
}


void gameUpdate(void) {
    g_pEngine->processCmds(handleInput);
}

void gameInit() {

    g_pEngine->setScene(""); // blank since set scene will just use test scene for now anyway
    entities = &g_pEngine->getEntities();
}


// IMPORTANT: Revise Engine API
