#include "dialog.hpp"
#include "ui.h"

#include <string_view>
#include <iostream>
#include <string>
#include "../event/game_event.hpp"

#define DIALOG_BOX_MAX_CHARS 255

enum PendingPrintOp {
    OP_NONE,
    OP_EXEC_BATTLE_START,
    OP_CONTINUE_PRINT
};

struct DialogState {
    std::string_view sourceStr;
    size_t           pos = 0;
    std::string      outStr;
    PendingPrintOp   pendingOp = OP_NONE;
};

DialogState state;

Entity* g_entity = {};

void setInteractableEntity(Entity* entity) {
    g_entity = entity;
}

std::unordered_map<std::string_view, PendingPrintOp> cmdMap = {
    {"battle_start",  OP_EXEC_BATTLE_START}
};

std::unordered_map<std::string_view, std::string> wordMap = {
    {"player_name", "Jack Bacon" /* placeholder name, will later be loaded from config */}
};

void handleCmdToken() {
    size_t endPos = state.sourceStr.find_first_of('$', state.pos + 1);
    std::string_view cmd = state.sourceStr.substr(state.pos + 1, endPos - 1);
    if (cmdMap.contains(cmd)) state.pendingOp = cmdMap[cmd];
    state.pos = endPos + 1; // continue char iteration from token end
}

void handleWordToken() {
    size_t endPos = state.sourceStr.find_first_of(']', state.pos + 1);
    std::string_view word = state.sourceStr.substr(state.pos + 1, ((endPos) - (state.pos + 1)));
    if (wordMap.contains(word)) state.outStr += wordMap[word]; // insert correct word
    state.pos = endPos + 1; // continue char iteration from token end
}


void advanceDialog() {

    /* First execute the pending op from last string */
    switch (state.pendingOp) {
        case OP_EXEC_BATTLE_START:
            //
            break;
        case OP_CONTINUE_PRINT:
            state.outStr.clear();
            break;
        case OP_NONE:
            break;
    }
    state.pendingOp = OP_NONE;

    while (state.pos < state.sourceStr.size() && state.outStr.size() < DIALOG_BOX_MAX_CHARS) {
        const char c = state.sourceStr[state.pos];
       // std::cout << c;
        switch(c) {
            case '$':
                handleCmdToken();
                /* early return, or the final line outside the loop will overwrite
                 * the pending op to OP_CONTINUE_PRINT instead of the correct one */
                return;
            case '[':
                handleWordToken();
                break;
            case '<':
                // TODO, this should also bypass OP_CONTINUE_PRINT. Any additional text after a dialogue option tag will be ignored
                break;
            default: // any other char
                state.outStr += c;
                ++state.pos;
                break;
        }
    }
    std::cout << "\n" << state.outStr << std::endl;
    setDialogText(state.outStr.data());
    state.pendingOp = OP_CONTINUE_PRINT; // when loop has terminated, await a UI_PROGRESS cmd
}



void initiateDialog() {
    // if (!entity->hasDialogString(getCurrentStoryEvent())) {} // ignore other string until they are implemented

    state.sourceStr = g_entity->getDialogString("default");
    state.pos = 0;

    advanceDialog(); // print initial piece of dialogue
    dialogBox();


}
