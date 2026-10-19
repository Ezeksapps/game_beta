#pragma once

// CURRENTLY COMPLETE:
// Renderer init
// glTF loader (Now complete with ability to handle more than one material)
// Sprite loader and renderer
// Graphics pipelines, render pass and frame buffer
// Sprite billboards' instance data
// window creation and input handling (needs to be associated to a callback)
// shaders for both pipelines
// Camera system
// sprite billboards' positioning relative to camera
// sprites owned by Entity objects (distinct types of entities such as NPCs will later extend that class)
// sprite sheet animation system, complete with directions
// sprite sheet cache
// ability to load sprite sheets with any frame size, as long as they are below the maximums defined in Renderer
// movement system, which syncs with movement animations
// Scene class, which loads the current scene and associated entities from a directory containing the scene's glTF and JSON
// Camera linked to player's movement and follows them around the map on all axes at a fixed offset
// UI system made with Nuklear and accompanying renderer
// input system that properly controls movement with expected behaviour (entity moves so long as movement key(s) are held)
// Collision detection between entities through a spatial hash system
// Entity dialogue system

// WIP: Collision detection (sptl hash done, quadtree + slope handling todo)
// Pokemon back-end mechanics (starting), P2P initial tests (class and base code setup, needs integration and testing)

// TODO: Remove Engine class, it isn't needed
// TODO: Alpha transparency for entities doesn't consider other entities (low pri)
// TODO: Decide on global game res and set diligent to use those dimensions (For consistent pixelated look), also use framebuffer resize callback
// TODO: Need to develop exit handling for Scenes

#include <cstdint>
#include <functional>

#include <json.hpp>

using json = nlohmann::json;

#include "entity/entity.hpp"
#include "scene/battle_scene.hpp"
#include "scene/world_scene.hpp"
#include "graphics/renderer.hpp"

/* --- GAME CORE FUNCS --- */

void gameInit();
void gameUpdate();

/* --- INPUT & CMD HANDLING --- */

/* NOTE: For correct movement behaviour, the enumerators must match the order of Direction's corresponding enumerators */
enum MovementCmd : uint8_t {
    ENTITY_MOVE_SOUTH       = 0,
    ENTITY_MOVE_SOUTH_EAST  = 1,
    ENTITY_MOVE_EAST        = 2,
    ENTITY_MOVE_NORTH_EAST  = 3,
    ENTITY_MOVE_NORTH       = 4,
    ENTITY_MOVE_NORTH_WEST  = 5,
    ENTITY_MOVE_WEST        = 6,
    ENTITY_MOVE_SOUTH_WEST  = 7,
    ENTITY_STOP_MOVEMENT    = 8,
};

enum GameCmd : uint8_t {

    UI_PROGRESS = 0,
    UI_ESCAPE   = 1,
    UI_OPEN     = 2,
    UI_DOWN     = 3,
    UI_UP       = 4

};

void initInputHandler();

void signalKeyPress(const int& keycode);
void signalKeyRelease(const int& keycode);

void handleMovement(MovementCmd& prevMovementCmd, MovementCmd& newMovementCmd);

// used to get the latest GameCmd in the command queue and pass it to the game's command callback
void processCmds(std::function<void(GameCmd cmd)> callback);

/* --- GLOBALS --- */

std::unique_ptr<Renderer> g_pRenderer = nullptr;
std::shared_ptr<Entity>   g_pPlayer = nullptr;

std::function<void(MovementCmd& prevMovementCmd, MovementCmd& newMovementCmd)> g_handleMovementCallback = nullptr;

