#include "game.hpp"
#include "engine/common.hpp"
#include "engine/engine.hpp"
#include "engine/entity/entity.hpp"

std::vector<std::shared_ptr<Entity>>* entities = nullptr;

void handleInput(GameCmd* input) {
    switch(*input) {
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


void handleMovement(MovementCmd* cmd) {
    switch(*cmd) {
        case ENTITY_MOVE_SOUTH:
            std::cout << "Moving South\n";
            (*entities)[0]->move(DIRECTION_SOUTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_SOUTH_EAST:
            std::cout << "Moving South East\n";
            (*entities)[0]->move(DIRECTION_SOUTH_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_EAST:
            std::cout << "Moving East\n";
            (*entities)[0]->move(DIRECTION_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH_EAST:
            std::cout << "Moving North East\n";
            (*entities)[0]->move(DIRECTION_NORTH_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH:
            std::cout << "Moving North\n";
            (*entities)[0]->move(DIRECTION_NORTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH_WEST:
            std::cout << "Moving North West\n";
            (*entities)[0]->move(DIRECTION_NORTH_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_WEST:
            std::cout << "Moving West\n";
            (*entities)[0]->move(DIRECTION_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_SOUTH_WEST:
            std::cout << "Moving South West\n";
            (*entities)[0]->move(DIRECTION_SOUTH_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_STOP_MOVEMENT:
            (*entities)[0]->endMovement();
            break;
    }
}


void gameUpdate(void) {
    g_pEngine->processCmds(handleInput);
    g_pEngine->handleMovement(handleMovement);
}

void gameInit() {

    g_pEngine->setScene("assets/scenes/debugScene");
    entities = &g_pEngine->getEntities();
}


// IMPORTANT: Revise Engine API
