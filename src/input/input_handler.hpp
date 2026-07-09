#pragma once

typedef enum GameCmd {
    ENTITY_MOVE_FORWARD,
    ENTITY_MOVE_LEFT,
    ENTITY_MOVE_RIGHT,
    ENTITY_MOVE_BACKWARD,
    ENTITY_RUN,
    UI_PROGRESS,
    UI_ESCAPE,
    UI_OPEN,
    SHORTCUT_BAG_OPEN,
    SHORTCUT_MAP_OPEN,
    SHORTCUT_PKMN_OPEN,
    ACTIVATE_DEBUG_OVERLAY,
    GAME_CAPTURE_SCREEN
} GameCmd;

void initInputHandler();
void handleInput(const int& keycode);
void processCmds(void (*callback)(GameCmd* cmd));
