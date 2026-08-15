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
// movement system, which syncs with movement animations

// FIX BUG: Camera system unstable and for some reason randomises its position every time program opened [high pri]
// FIX BUG: Camera functions for rotation do work but do not apply properly as they are in Renderer for some reason [med pri]

// ISSUE: Entity positions are treated by the geometry shader as the centre position, which means only half the sprite billboard is above the floor (Z-axis)
// and that the sprites do not properly align with the grid (X & Y axes), figure out how the grid will be dealt with

// TODO (low pri): Make entity translation system use lerp instead

// NEXT TODO: Sprite sheets (from PMD repo) have different scales, make them appear the same size when rendering
// FIX: GLFW only detecting keydown event and sending one command for movement. This is wrong, movement should continue afrer keydown
// until key is released (dont stop mid movement though, that violates grid system, stop at nearest tile (ceil))

/* CURRENT STATUS:
 * Compiles successfully, renderer and pipelines initialise with no issues.
 * Map pipeline renders 3D environment. Camera system is properly set up and can rotate with yaw, pitch and roll
 * Sprite pipeline renders sprite with proper UVs and alpha channels and properly matches the camera's rotation to appear flat.
 * Sprite sheet system fully implemented, but needs to accomodate directionless sprite sheets as well.
 * Entitiy movement operational, corresponding movement animation plays with it and is synced so that moving one tile will run one exactly cycle
 * of that animation
 */

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit();
