#include "ui.h"
#include "text_display.hpp"

#include <string_view>
#include <unordered_map>
#include <string>
#include "../event/game_event.hpp"

#define TEXT_DISPLAY_MAX_CHARS 255

enum PendingPrintOp {
    OP_NONE,
    OP_EXEC_BATTLE_START,
    OP_CONTINUE_PRINT
};

struct TextState {
    std::string_view sourceStr;
    size_t           pos = 0;
    std::string      outStr;
    PendingPrintOp   pendingOp = OP_NONE;
};

TextState state;

std::unordered_map<std::string_view, PendingPrintOp> cmdMap = {
    {"battle_start",  OP_EXEC_BATTLE_START}
};

std::unordered_map<std::string_view, std::string> wordMap = {
    {"player_name", "Jack Bacon" /* placeholder name, will later be loaded from config */}
};


void handleCmdToken() {
    size_t endPos = state.sourceStr.find_first_of('$', state.pos + 1);
    std::string_view cmd = state.sourceStr.substr(state.pos + 1, ((endPos) - (state.pos + 1)));

    if (cmdMap.contains(cmd)) state.pendingOp = cmdMap[cmd];
    state.pos = endPos + 1; // continue char iteration from token end
}

void handleWordToken() {
    size_t endPos = state.sourceStr.find_first_of(']', state.pos + 1);
    std::string_view word = state.sourceStr.substr(state.pos + 1, ((endPos) - (state.pos + 1)));
    if (wordMap.contains(word)) state.outStr += wordMap[word]; // insert correct word
    state.pos = endPos + 1; // continue char iteration from token end
}


void advanceText() { // aka execute the pending print op

    /* First execute the pending op from last string */
    switch (state.pendingOp) {
        case OP_EXEC_BATTLE_START:
            triggerEvent(GAME_EVENT_BATTLE_START);
            break;
        case OP_CONTINUE_PRINT:
            state.outStr.clear();
            break;
        case OP_NONE:
            break;
    }
    state.pendingOp = OP_NONE; // reset

    while (state.pos < state.sourceStr.size() && state.outStr.size() < TEXT_DISPLAY_MAX_CHARS) {
        const char c = state.sourceStr[state.pos];
        switch(c) {
            case '$':
                handleCmdToken();
                /* early return, or the final line outside the loop will overwrite
                 * the pending op to OP_CONTINUE_PRINT instead of the correct one */
                setDisplayText(state.outStr.data());
                return;
            case '[':
                handleWordToken();
                break;
            case '<':
                // TODO, this should also bypass OP_CONTINUE_PRINT. Any additional text after a dialogue option tag will be ignored
                break;
            case '\n':
                break;
            default: // any other char
                state.outStr += c;
                ++state.pos;
                break;
        }
    }

    setDisplayText(state.outStr.data());
    state.pendingOp = OP_CONTINUE_PRINT; // when loop has terminated, await a UI_PROGRESS cmd
}
