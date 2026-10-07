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

// separation of game and engine code (DEPRECATED, to be reverted, this distinction doesn't matter rn)

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

// WIP: Collision detection (sptl hash done, quadtree + slope handling todo),
// Pokemon back-end mechanics (starting), P2P initial tests (class and base code setup, needs integration and testing)

// CHECK: Why is my stylesheet loader broken

// TODO: Remove Engine class, it isn't needed
// TODO: Alpha transparency for entities doesn't consider other entities (low pri)
// TODO: Decide on global game res and set diligent to use those dimensions (For consistent pixelated look), also use framebuffer resize callback
// TODO: Make UI skin (also add missing glyphs to font)
// TODO: NPC interactions (now possible w/ sptl hash)
// TODO: Need to develop exit handling for Scenes

#include "engine/engine.hpp"

void gameUpdate(void);
void gameInit();
