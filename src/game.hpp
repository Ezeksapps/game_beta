#pragma once

// TODO:

// [Engine code]
// set one entity as the player, others as their corresponding subclass of entity
// Collision logic for 3D space (stairs will always be at a fixed angle, so any angle > stair angle = impassable)
// Create dialogue system

// CURRENTLY COMPLETE:
// Renderer init
// glTF loader
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

// WIP: Pokemon back-end mechanics (starting), UI renderer (mostly complete), P2P initial tests (class and base code setup, needs integration and testing)

// ISSUE: Entity positions are treated by the geometry shader as the centre position, which means only half the sprite billboard is above the floor (Z-axis)
// and that the sprites do not properly align with the grid (X & Y axes), figure out how the grid will be dealt with

// FIX (med pri): movement behaviour doesn't match expected (stop moving when not holding any movement key), maybe cmd queue not best format?

// TODO: Revise which function belong in the engine as opposed to game code (most things currently being shoved in Engine)
// TODO (low pri): Make entity translation system use lerp instead
// TODO: Decide on global game res and set diligent to use those dimensions (For consistent pixelated look), also use framebuffer resize callback

// TODO: Make UI skin (also add missing glyphs to font)

// TODO: COLLISION DETECTION (HIGH PRI, needed for most mechanics)

/* CURRENT STATUS:
 * Compiles successfully, renderer and pipelines initialise with no issues.
 * Map pipeline renders 3D environment. Camera system is properly set up and can rotate with yaw, pitch and roll
 * Sprite pipeline renders sprite with proper UVs and alpha channels and properly matches the camera's rotation to appear flat.
 * Sprite sheet system fully implemented, but needs to accomodate directionless sprite sheets as well.
 * All sprite sheets for an entity are cached on load, so the texture data can be switched as needed, meaning no visual lag is seen when switching
 * between sprite sheets. Frame sizes in sprite sheets may be of any size below the maximum dimensions defined in Renderer.
 * Entitiy movement operational, corresponding movement animation plays with it and is synced so that moving one tile will run one exactly cycle
 * of that animation
 */

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit();
