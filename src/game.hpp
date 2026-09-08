#pragma once

// TODO:

// [Engine code]
// set one entity as the player, others as their corresponding subclass of entity
// Collision logic for 3D space (stairs will always be at a fixed angle, so any angle > stair angle = impassable)
// Create dialogue system

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
// separation of game and engine code
// sprites owned by Entity objects (distinct types of entities such as NPCs will later extend that class)
// sprite sheet animation system, complete with directions
// sprite sheet cache
// ability to load sprite sheets with any frame size, as long as they are below the maximums defined in Renderer
// movement system, which syncs with movement animations
// Scene class, which loads the current scene and associated entities from a directory containing the scene's glTF and JSON
// Camera linked to player's movement and follows them around the map on all axes at a fixed offset
// UI system made with Nuklear and accompanying renderer (in testing)
// input system that properly controls movement with expected behaviour (entity moves so long as movement key(s) are held)

// WIP: Pokemon back-end mechanics (starting), P2P initial tests (class and base code setup, needs integration and testing)

// FIX (med pri): Entity positions are treated by the geometry shader as the centre position, which means only half the sprite billboard is above the floor (Z-axis)
// FIX (high pri): UI pipeline works and draw commands are dispatched and executed properly, but no UI is visible because view matrix isnt accounted for

// TODO: Revise which function belong in the engine as opposed to game code (most things currently being shoved in Engine)
// TODO: Decide on global game res and set diligent to use those dimensions (For consistent pixelated look), also use framebuffer resize callback
// TODO: Make UI skin (also add missing glyphs to font)
// TODO: COLLISION DETECTION (HIGH PRI, needed for most mechanics)
// TODO: Bind all textures for scene renderer (may need tex array) [HIGH PRI]
// TODO: Need more complex test scene for development of scene JSON spec

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit();
