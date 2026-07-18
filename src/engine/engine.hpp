#pragma once

/* Engine holds the instances of all core game systems as well as important data such as the currently loaded Scene
 * all files include common.hpp, which includes the global singleton instance of the Engine. */

#include "common.hpp"
#include "graphics/renderer.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include <memory>

enum GameCmd {
    ENTITY_MOVE_FORWARD,
    ENTITY_MOVE_LEFT,
    ENTITY_MOVE_RIGHT,
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

    /* --- INPUT HANDLING --- */

    // Send a keycode (GLFW enum key) to the input handler, the corresponding GameCmd be added to command queue
    // This is for the use of the platform entry point's keypress callback.
    // The game code should attach its own callback to processCmds() to define what happens when each GameCmd is recieved
    void handleInput(const int& keycode);

    // used to get the latest command in the command queue and pass it to the game's command callback
    void processCmds(void (*callback)(GameCmd* cmd));

    /* --- RENDERING --- */

    // This just acts as a public-access way to call renderer's renderFrame()
    void renderFrame();

private:

    std::unique_ptr<Scene> m_pScene;
    std::unique_ptr<Renderer> m_pRenderer;

};
