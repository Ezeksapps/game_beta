#pragma once

/* Engine holds the instances of all core game systems as well as important data such as the currently loaded Scene
 * all files include common.hpp, which includes the global singleton instance of the Engine. */

#include "common.hpp"
#include "graphics/renderer.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include <memory>

/* -- OLD
enum GameCmd : uint8_t {
    ENTITY_MOVE_FORWARD,
    ENTITY_MOVE_LEFT,     // west
    ENTITY_MOVE_RIGHT,    // east
    ENTITY_MOVE_BACKWARD,
    ENTITY_RUN,
    UI_PROGRESS,
    UI_ESCAPE,
    UI_OPEN,
    SHORTCUT_BAG_OPEN,
    SHORTCUT_MAP_OPEN,
    SHORTCUT_PKMN_OPEN,
    ACTIVATE_DEBUG_OVERLAY,
    GAME_CAPTURE_SCREEN
};*/

enum GameCmd : uint8_t {
    // directional commands match order of directions for Entity's enum
    ENTITY_FACE_SOUTH       = 0,
    ENTITY_FACE_SOUTH_EAST  = 1,
    ENTITY_FACE_EAST        = 2,
    ENTITY_FACE_NORTH_EAST  = 3,
    ENTITY_FACE_NORTH       = 4,
    ENTITY_FACE_NORTH_WEST  = 5,
    ENTITY_FACE_WEST        = 6,
    ENTITY_FACE_SOUTH_WEST  = 7,

    ENTITY_MOVE_SOUTH      = 8,
    ENTITY_MOVE_SOUTH_EAST = 9,
    ENTITY_MOVE_EAST       = 10,
    ENTITY_MOVE_NORTH_EAST = 11,
    ENTITY_MOVE_NORTH      = 12,
    ENTITY_MOVE_NORTH_WEST = 13,
    ENTITY_MOVE_WEST       = 14,
    ENTITY_MOVE_SOUTH_WEST = 15,

    // running will be implemented post-release

    UI_PROGRESS = 16,
    UI_ESCAPE   = 17,
    UI_OPEN     = 18,

    // termination commands for continuous actions, sent when releasing keys assigned to continuous actions

    ENTITY_STOP_MOVEMENT = 20
};

struct EngineConfig {
    uint32_t viewportWidth;
    uint32_t viewportHeight;
    Diligent::NativeWindow window;
    Diligent::RENDER_DEVICE_TYPE renderBackend;
};

class Engine {

public:

    Engine(const EngineConfig& config);
    ~Engine();

    // all scenes are in their own directory. setScene should take the name of the directory, so that
    // it can locate the JSON and glTF files corresponding to the scene and create the object
    // acts as a public-access way to call renderer's setScene()
    void setScene(const std::string& sceneDir);

    /* --- GETTERS --- */

    std::vector<std::shared_ptr<Entity>>& getEntities();

    /* --- INPUT HANDLING --- */

    // Send a keycode (GLFW enum key) to the input handler
    void handleInput(const int& keycode);

    void signalKeyPress(const int& keycode);
    void signalKeyRelease(const int& keycode);

    // used to get the latest command in the command queue and pass it to the game's command callback
    void processCmds(std::function<void(GameCmd* cmd)> callback);

    /* --- RENDERING --- */

    // This just acts as a public-access way to call renderer's renderFrame()
    void renderFrame();

    std::unique_ptr<Renderer> m_pRenderer; // TEMP MEASURE!

private:

    //std::unordered_map<GameCmd, KeypressType>


};
