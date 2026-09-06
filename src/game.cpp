#include "game.hpp"
#include "engine/common.hpp"
#include "engine/engine.hpp"
#include "engine/entity/entity.hpp"

std::vector<std::shared_ptr<Entity>>* entities = nullptr;

void handleInput(GameCmd& input) {
    switch(input) {
        case UI_PROGRESS:
            break;
        case UI_ESCAPE:
            // send message pop back UI stack to handler, ignored if no UI elem(s) active
            break;
        case UI_OPEN:
            g_pEngine->m_pRenderer->testUiSystem();
            // pauseMenu(), only used in overworld
            break;
    }
}

/* NOTE: MovementCmd can be directly cast to a Direction, since the order of enumerators match between the enumerated types */
void handleMovement(MovementCmd& prevMovementCmd, MovementCmd& newMovementCmd) {
    if (prevMovementCmd == ENTITY_STOP_MOVEMENT) {
        if (newMovementCmd == prevMovementCmd) return; // ignore repeat stop command, entity already stationary
        // otherwise entity should begin movement
        else (*entities)[0]->move((Direction)newMovementCmd, ANIM_EVENT_WALK);
    }
    // if reaching here, then prev cmd was a movement command
    else if (newMovementCmd == ENTITY_STOP_MOVEMENT) (*entities)[0]->endMovement();
    // if here, prev cmd was movement and new command is not to stop, therefore we must be changing direction
    else (*entities)[0]->changeMovementDirection((Direction)newMovementCmd);
}

void gameUpdate(void) {
    g_pEngine->processCmds(handleInput);

}

void gameInit() {
    g_pEngine->setScene("assets/scenes/debugScene");
    entities = &g_pEngine->getEntities();
    g_pEngine->setHandleMovementCallback(handleMovement);
}


// IMPORTANT: Revise Engine API
