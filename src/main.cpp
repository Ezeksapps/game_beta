/* Program entry point for GNU/Linux & Windows builds */

#include "common.hpp"
#include "game_state.hpp"

#include "game.hpp"
#include "graphics/renderer.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "input/input_handler.hpp"

#include <iostream>
#include <stdexcept>

/* size at which initial window is created */
const uint32_t WIDTH_INITIAL = 800;
const uint32_t HEIGHT_INITIAL = 600;

GLFWwindow* window;
Diligent::NativeWindow wnd;

std::shared_ptr<GameState> g_state = std::make_shared<GameState>();

Diligent::RENDER_DEVICE_TYPE renderBackend;


void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    g_state->g_renderer->m_windowWidth = width;
    g_state->g_renderer->m_windowHeight = height;
    //renderer->framebufferResized = true;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        //handleInput(key);  // Send raw keycode
        switch (key) {
            case GLFW_KEY_A:
                g_state->g_renderer->m_pCamera->rotate(vec3(0.0f, -(std::numbers::pi_v<float> * 0.01), 0.0f));
                break;
            case GLFW_KEY_D:
                g_state->g_renderer->m_pCamera->rotate(vec3(0.0f, (std::numbers::pi_v<float> * 0.01), 0.0f));
            case GLFW_KEY_W:
                g_state->g_renderer->m_pCamera->rotate(vec3(-(std::numbers::pi_v<float> * 0.01), 0.0f, 0.0f));
                break;
            case GLFW_KEY_S:
                g_state->g_renderer->m_pCamera->rotate(vec3((std::numbers::pi_v<float> * 0.01), 0.0f, 0.0f));
                break;
            default:
                break;
        }
    }
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

        g_state->g_renderer->renderFrame();
        gameUpdate();
    }
}

/* Entry point */
int main() {
    try {
        g_state->g_renderer = std::make_unique<Renderer>(WIDTH_INITIAL, HEIGHT_INITIAL);
        initWindow();

        g_state->g_renderer->initRenderer(wnd, renderBackend);
        g_state->g_renderer->loadGLB("assets/test.glb");
        g_state->g_renderer->loadSprite("assets/sprites/eevee.png");
       // renderer->loadSprite("assets/sprites/vulpix_fire.png");

        mainLoop();

        glfwDestroyWindow(window);
        glfwTerminate();

    } catch (const std::exception& e) {
        std::cout << "----\n[!] Program About to exit, exception caught!";
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


