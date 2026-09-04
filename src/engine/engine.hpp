#pragma once

/* Engine holds the instances of all core game systems as well as important data such as the currently loaded Scene
 * all files include common.hpp, which includes the global singleton instance of the Engine. */

#include "common.hpp"
#include "graphics/renderer.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include <memory>


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
    void stopCurrentMovement();
    void updateMovement();

    // used to get the latest GameCmd in the command queue and pass it to the game's command callback
    void processCmds(std::function<void(GameCmd* cmd)> callback);
    void handleMovement(std::function<void(MovementCmd* cmd)> callback);

    /* --- RENDERING --- */

    // This just acts as a public-access way to call renderer's renderFrame()
    void renderFrame();

    std::unique_ptr<Renderer> m_pRenderer; // TEMP MEASURE!

private:

    //std::unordered_map<GameCmd, KeypressType>


};
