#pragma once

// TODO:

// [Engine code]
// create system to move sprite billboards and synchronise the movement to the sprite animation
// set one entity as the player, others as NPCs (Likely classes extending Entity)
// Collision logic for 3D space (stairs will always be at a fixed angle, so any angle > stair angle = impassable)
// X & Y coords should work like a 2D grid system
// Along w/ GLB, map also defined by a JSON giving NPCs in map and exits/links to other areas within map
// Create dialogue system and UI Renderer (defined by XML reader, then render in here, maybe in separate subpass?)

// [Game code]
// Create other systems, assign to game events

// CURRENTLY COMPLETE:
// Renderer init
// GLB loader
// Sprite loader and renderer
// Graphics pipelines, render pass and frame buffer
// Sprite billboards' vertex, index and instance buffers
// window creation and input handling (needs to be associated to a callback)
// shaders for both pipelines
// Camera system
// sprite billboards' positioning relative to camera
// separate game and engine code
// sprite sheet animation system, complete with directions

// FIX BUG: Camera system unstable and for some reason randomises its position every time program opened
// FIX BUG: Camera functions for rotation do work but do not apply properly as they are in Renderer for some reason

// ISSUE: Entity positions are treated by the geometry shader as the centre position, which means only half the sprite billboard is above the floor (Z-axis)
// and that the sprites do not properly align with the grid (X & Y axes), figure out how the grid will be dealt with

// NEXT TODO: Movement -- IN PROGRESS APPROX 90%
// CHECK: Movement system mostly complete, but program now crashes on launch, investigate why

/* CURRENT STATUS:
 * Compiles successfully, renderer and pipelines initialise with no issues.
 * Map pipeline renders 3D environment. Camera system is properly set up and can rotate with yaw, pitch and roll
 * Sprite pipeline renders sprite with proper UVs and alpha channels and properly matches the camera's rotation to appear flat.
 * Sprite sheet system fully implemented, but needs to accomodate directionless sprite sheets as well
 */

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit();
