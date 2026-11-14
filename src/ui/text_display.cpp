#include "ui.h"
#include "text_display.hpp"
#include "../event/game_event.hpp"

#define TEXT_DISPLAY_MAX_CHARS 255

enum PendingPrintOp {
    OP_NONE,
    OP_EXEC_BATTLE_START,
    OP_CONTINUE_PRINT
};

struct TextState {
    std::string sourceStr;
    size_t           pos = 0;
    std::string      outStr;
    PendingPrintOp   pendingOp = OP_NONE;
};

TextState state;

std::unordered_map<std::string, PendingPrintOp> cmdMap = {
    {"battle_start",  OP_EXEC_BATTLE_START}
};

std::unordered_map<std::string, std::string> wordMap = {
    // constant vals
    {"pr1_f", ""},
    {"pr2_f", ""},
    {"pr1_m", ""},
    {"pr2_m", ""}
    // push savedata-dependent vals here
};

// not used unless supplyExternalWordMap() is called
std::unordered_map<std::string, std::string> externalWordMap;


/*  'word' may need a better name
 *   the primary map is kept inside the text display system. This map contains words common to all text displays
 *   and that are constant throughout the game (not dependent on temporary states)
 *   For any words that are not in the central map (because they are temporary or non-constant),
 *   an external map must be supplied. When a word is looked up, if it isn't found in the primary map, then the secondary map should be checked
 *   Since the maps are unordered STL maps, these lookups aren't expensive, so efficiency is preserved.
 *
 *   The primary word map is loaded when the game is initially started based on the player save and other config files
 */

void initWordGlobalMappings(const json& saveDataJson) {
    // prepare read of JSON asset(s)
    wordMap.emplace("player_name", saveDataJson["name"]);

}

void supplyExternalWordMap(std::unordered_map<std::string, std::string>& map) { externalWordMap = map; }

std::string isolateToken(const char endSymbol) {
    size_t endPos = state.sourceStr.find_first_of(endSymbol, state.pos + 1);
    std::string token = state.sourceStr.substr(state.pos + 1, ((endPos) - (state.pos + 1)));
    state.pos = endPos + 1; // continue char iteration from token end
    return token;
}

void handleCmdToken() {
    std::string cmd = isolateToken('$');
    if (cmdMap.contains(cmd)) state.pendingOp = cmdMap[cmd];
    else state.outStr += "UNRECOGNISED CMD";
}

void handleWordToken() {
    std::string word = isolateToken(']');
    if (wordMap.contains(word)) state.outStr += wordMap[word]; // insert correct word
    else if (externalWordMap.size() > 0 && externalWordMap.contains(word)) state.outStr += externalWordMap[word];
    else state.outStr += "UNKNOWN";
}

void setSourceString(const std::string& str) { // reset state with new string
    state = TextState {
        .sourceStr = str,
        .pos = 0,
        .outStr = "",
        .pendingOp = OP_NONE
    };

    advanceText(); // print initial piece of text
}

void setSourceStringf(const std::string& str, std::vector<std::string> replacements) {
    /* The format of strings passed to this function is similar to that of C's printf function
     * % is used to indicate where one of the provided external strings should be insterted.
     * This is done in the order that the external strings are passed
     */

    state = TextState { // reset state first
        .sourceStr = "",
        .pos = 0,
        .outStr = "",
        .pendingOp = OP_NONE
    };

    // generate source string

    int i = 0;
    for (size_t pos = 0; pos < str.size(); ++pos) {
        char c = str[pos];

        if (c == '%') state.sourceStr += replacements[i++];
        else state.sourceStr += c;
    }

    advanceText(); // print initial piece of text
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

    /*if (state.pos == state.sourceStr.size()) { // Works for overworld entity dialogue, doesn't work when displaying battle strings?
        popFromStack();

        state = TextState { // reset state
            .sourceStr = "",
            .pos = 0,
            .outStr = "",
            .pendingOp = OP_NONE
        };
    }*/

    // TODO: tidy this a bit
    while (state.pos < state.sourceStr.size() && state.outStr.size() < TEXT_DISPLAY_MAX_CHARS) {
        char c = state.sourceStr[state.pos];
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
            //case '\n':
                // Test showed Nuklear does not autmatically handle newline chars TODO: Add proper handling!
                //break;
            default: // any other char
                state.outStr += c;
                ++state.pos;
                break;
        }
    }

    // TODO: add way to identify when all text has been printed and therefore when to close the display

    setDisplayText(state.outStr.data());
    state.pendingOp = OP_CONTINUE_PRINT; // when loop has terminated, await a UI_PROGRESS cmd
}
