#include "game.hpp"
#include "engine/common.hpp"
#include "engine/engine.hpp"
#include "engine/entity/entity.hpp"
#include <iostream>

std::vector<std::shared_ptr<Entity>>* entities = nullptr;

void handleInput(GameCmd* input) {
    switch(*input) {
        case ENTITY_FACE_SOUTH:
            (*entities)[0]->m_direction = DIRECTION_SOUTH;
            break;
        case ENTITY_FACE_SOUTH_EAST:
            (*entities)[0]->m_direction = DIRECTION_SOUTH_EAST;
            break;
        case ENTITY_FACE_EAST:
            (*entities)[0]->m_direction = DIRECTION_EAST;
            break;
        case ENTITY_FACE_NORTH_EAST:
            (*entities)[0]->m_direction = DIRECTION_NORTH_EAST;
            break;
        case ENTITY_FACE_NORTH:
            (*entities)[0]->m_direction = DIRECTION_NORTH;
            break;
        case ENTITY_FACE_NORTH_WEST:
            (*entities)[0]->m_direction = DIRECTION_NORTH_WEST;
            break;
        case ENTITY_FACE_WEST:
            (*entities)[0]->m_direction = DIRECTION_WEST;
            break;
        case ENTITY_FACE_SOUTH_WEST:
            (*entities)[0]->m_direction = DIRECTION_SOUTH_WEST;
            break;
        case ENTITY_MOVE_SOUTH:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_SOUTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_SOUTH_EAST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_SOUTH_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_EAST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH_EAST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_NORTH_EAST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_NORTH, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_NORTH_WEST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_NORTH_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_WEST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_WEST, ANIM_EVENT_WALK);
            break;
        case ENTITY_MOVE_SOUTH_WEST:
            if (!(*entities)[0]->isMoving()) (*entities)[0]->move(DIRECTION_SOUTH_WEST, ANIM_EVENT_WALK);
            break;
        case UI_PROGRESS:
            break;
        case UI_ESCAPE:
            // send message pop back UI stack to handler, ignored if no UI elem(s) active
            break;
        case UI_OPEN:
            g_pEngine->m_pRenderer->testUiSystem();
            // pauseMenu(), only used in overworld
            break;
        case ENTITY_STOP_MOVEMENT:
             std::cout << "Stop movement cmd detected\n";
             (*entities)[0]->endMovement();
             break;
    }
}


void gameUpdate(void) {
    g_pEngine->processCmds(handleInput);
}

void gameInit() {

    g_pEngine->setScene("assets/scenes/debugScene");
    entities = &g_pEngine->getEntities();
}


// IMPORTANT: Revise Engine API
