#include "game.hpp"
#include "common.hpp"

#include "ui/ui.h"

void handleInput(GameCmd input) {
    switch(input) {
        case UI_PROGRESS:
            std::cout << "PROGRESS_UI\n";
            progressUI();
            break;
        case UI_ESCAPE:
            popFromStack();
            // send message pop back UI stack to handler, ignored if no UI elem(s) active
            break;
        case UI_OPEN:
            pauseMenu();
            // pauseMenu(), only used in overworld
            break;
        case UI_DOWN:
            down();
            break;
        case UI_UP:
            up();
            break;
    }
}

void gameUpdate(void) {
    processCmds(handleInput);
}

/* NOTE: MovementCmd can be directly cast to a Direction, since the order of enumerators match between the enumerated types */
void handleMovement(MovementCmd& prevMovementCmd, MovementCmd& newMovementCmd) {
    if (prevMovementCmd == ENTITY_STOP_MOVEMENT) {
        if (newMovementCmd == prevMovementCmd) return; // ignore repeat stop command, entity already stationary
        // otherwise entity should begin movement
        else g_pPlayer->move((Direction)newMovementCmd, ANIM_EVENT_WALK);
    }
    // if reaching here, then prev cmd was a movement command
    else if (newMovementCmd == ENTITY_STOP_MOVEMENT) g_pPlayer->endMovement();
    // if here, prev cmd was movement and new command is not to stop, therefore we must be changing direction
    else g_pPlayer->changeMovementDirection((Direction)newMovementCmd);
}
