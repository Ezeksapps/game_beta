#pragma once

// TODO:

// Animate sprites w/ sprite sheet system -- IN PROGRESS, APPROX 70% COMPLETE

// [Engine code]
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

// FIX BUG: Viewport flashes (likely enough to trigger photosensitivity), possibly due to unused swap chain image

/* CURRENT STATUS:
 * Compiles successfully, renderer and pipelines initialise with no issues.
 * Map pipeline renders with no issues. Camera system is properly set up and can rotate with yaw, pitch and roll with no issues
 * Sprite pipeline renders sprite with proper UVs and alpha channels and properly matches the camera's rotation to appear flat.
 */

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit(const std::unique_ptr<Engine>& pEngineRef);
