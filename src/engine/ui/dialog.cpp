#include "ui.h"

#include <string_view>
#include <string>
#include <memory>
#include "../entity/entity.hpp"
#include "../event/game_event.hpp"

#define DIALOG_BOX_MAX_CHARS 25

enum PendingPrintOp {
    OP_NONE,
    OP_EXEC_BATTLE_START,
    OP_CONTINUE_PRINT
};

std::unordered_map<std::string_view, PendingPrintOp> cmdMap = {
    {"battle_start",  OP_EXEC_BATTLE_START}
};

std::unordered_map<std::string_view, std::string> wordMap = {
    {"player_name", "Jack Bacon" /* placeholder name, will later be loaded from config */}
};

std::string g_outStr = "";
PendingPrintOp g_pendingOp = OP_NONE;

// forward decls
void handleCmdToken(const std::string_view& str, size_t& pos);
void handleWordToken(const std::string_view& str, size_t& pos);

void loopDialogString(const std::string_view& sourceStr, size_t& pos) {
    g_pendingOp = OP_NONE;
    while (pos < sourceStr.size() && g_outStr.size() > DIALOG_BOX_MAX_CHARS) {
        const char c = sourceStr[pos];
        switch(c) {
            case '$':
                handleCmdToken(sourceStr, pos);
                /* early return, or the final line outside the loop will overwrite
                 * the pending op to OP_CONTINUE_PRINT instead of the correct one */
                return;
            case '[':
                handleWordToken(sourceStr, pos);
                break;
            case '<':
                // TODO
                break;
            default: // any other char
                g_outStr += c;
                break;
        }
        ++pos;
    }
    g_pendingOp = OP_CONTINUE_PRINT; // when loop has terminated, await a UI_PROGRESS cmd
}

void handleCmdToken(const std::string_view& str, size_t& pos) {
    size_t endPos = str.find_first_of('$', pos + 1);
    std::string_view cmd = str.substr(pos + 1, endPos - 1);
    if (cmdMap.contains(cmd)) g_pendingOp = cmdMap[cmd];
    pos = endPos; // continue char iteration from token end
}

void handleWordToken(const std::string_view& str, size_t& pos) {
    size_t endPos = str.find_first_of(']', pos + 1);
    std::string_view word = str.substr(pos + 1, endPos - 1);
    if (wordMap.contains(word)) g_outStr += wordMap[word]; // insert correct word
    pos = endPos; // continue char iteration from token end
}

void initiateDialog(const std::shared_ptr<Entity>& entity) {
    // if (!entity->hasDialogString(getCurrentStoryEvent())) {}

    std::string_view sourceStr = entity->getDialogString("default");
    size_t pos = 0;

    dialogLoop:
    loopDialogString(sourceStr, pos);
    // when this function has finished, either the entire string has been read
    // or there is a pending operation, handle those operations here
    switch (g_pendingOp) {
        case OP_NONE:
            return; // dialogue is complete
        case OP_CONTINUE_PRINT:
            // await UI_PROGRESS then...
            goto dialogLoop;
            break;
        case OP_EXEC_BATTLE_START:
            // await UI_PROGRESS then...
            break;
    }
}
