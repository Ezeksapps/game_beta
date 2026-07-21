#include "game.hpp"
#include "engine/common.hpp"
#include <iostream>

std::vector<std::shared_ptr<Entity>> entities;

void handleInput(GameCmd* input) {
    switch(*input) {
        case ENTITY_MOVE_FORWARD:
            std::cout << "W pressed\n";
            break;
        case ENTITY_MOVE_LEFT:

            break;
        case ENTITY_MOVE_RIGHT:

            break;
        case ENTITY_MOVE_BACKWARD:

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
    //entities = g_pEngine->getEntities();
}
