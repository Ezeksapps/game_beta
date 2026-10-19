/* Program entry point for GNU/Linux & Windows builds */

#include "common.hpp"
#include "game.hpp"

#include "fileio.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

/* size at which initial window is created */
const uint32_t WIDTH_INITIAL = 480;
const uint32_t HEIGHT_INITIAL = 270;

GLFWwindow* window;
Diligent::NativeWindow wnd;
Diligent::RENDER_DEVICE_TYPE renderBackend;

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    //g_state->g_renderer->m_windowWidth = width;
    //g_state->g_renderer->m_windowHeight = height;
    //renderer->framebufferResized = true;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   // std::cout << key << std::endl;
    if (action != GLFW_RELEASE) signalKeyPress(key);
    else signalKeyRelease(key);
}

void initWindow() {
    if (glfwInit() != GLFW_TRUE) return;

    if (glfwVulkanSupported()) {
        renderBackend = Diligent::RENDER_DEVICE_TYPE_VULKAN;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // set GLFW to not create an OpenGL Instance for this window
    }
    else { // No Vulkan support (probably pre-2017 hardware), fallback to OpenGL 3.3
        renderBackend = Diligent::RENDER_DEVICE_TYPE_GL;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // Set window as resizable

    window = glfwCreateWindow(WIDTH_INITIAL, HEIGHT_INITIAL, "Program", nullptr, nullptr);

#if defined (__linux__)
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) { // Wayland
        wnd.pWaylandSurface = glfwGetWaylandWindow(window);
        wnd.pDisplay = glfwGetWaylandDisplay();
    }
    else { // else X11
        wnd.WindowId = glfwGetX11Window(window);
        wnd.pDisplay = glfwGetX11Display();
    }
#elif defined (_WIN32)
#endif

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    glfwShowWindow(window);
    glfwSetKeyCallback(window, keyCallback);
}

/* Window event loop. Events are handled by GLFW */
void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

         g_pRenderer->renderFrame();
         gameUpdate();
    }
}

void gameInit() {
    // combine to one func?
    g_pRenderer = std::make_unique<Renderer>(WIDTH_INITIAL, HEIGHT_INITIAL);
    g_pRenderer->initRenderer(wnd, renderBackend);
    // TODO: renderer needs to re-create swap chain when viewport size changed
    initInputHandler();

    // load player/save data (will later be relocated to separate func in dedicated save/load system):

    json playerData = json::parse(readJsonAsset("savedata/playerinfo.json"));

    g_pPlayer = std::make_shared<Entity>(playerData["sprites"], 0);
    g_pPlayer->m_pos = vec3(playerData["posX"], playerData["posY"], 0.5f);

    std::unique_ptr<Scene> scene = std::make_unique<WorldScene>(playerData["lastScene"], g_pPlayer);
    g_pRenderer->setScene(scene);

    g_handleMovementCallback = handleMovement;
}

/* Entry point */
int main() {
    try {
        initWindow();

        gameInit();

        mainLoop();

        glfwDestroyWindow(window);
        glfwTerminate();

    } catch (const std::exception& e) {
        std::cout << "---------\n[!] Program About to exit, exception caught!\n";
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


