#include "game.hpp"
#include <iostream>

std::unique_ptr<Engine>& g_pEngineRef;

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
    g_pEngineRef->processCmds(handleInput);
}

void gameInit(const std::unique_ptr<Engine>& pEngineRef) {
    //SpriteSheetInfo playerSpriteSheetInfo {
    //    .entityEvents = {ANIM_EVENT_WALK},
    //    .spriteSheets = {"eevee_walk.png"},
   //     .eventPages = {7}
   // };
  //  Entity* player = new Entity(playerSpriteSheetInfo);
    g_pEngineRef = pEngineRef;

}
