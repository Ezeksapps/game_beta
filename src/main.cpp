/* Program entry point for GNU/Linux & Windows builds */

#include "engine/common.hpp"
#include "game.hpp"

#include <DiligentCore/Platforms/interface/NativeWindow.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

/* size at which initial window is created */
const uint32_t WIDTH_INITIAL = 800;
const uint32_t HEIGHT_INITIAL = 600;

GLFWwindow* window;
Diligent::NativeWindow wnd;
Diligent::RENDER_DEVICE_TYPE renderBackend;

std::unique_ptr<Engine> g_pEngine;

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    //g_state->g_renderer->m_windowWidth = width;
    //g_state->g_renderer->m_windowHeight = height;
    //renderer->framebufferResized = true;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        g_pEngine->handleInput(key);  // Send raw keycode
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

         g_pEngine->renderFrame();
         gameUpdate();
    }
}

/* Entry point */
int main() {
    try {
        initWindow();

        EngineConfig config = {
            .viewportWidth = WIDTH_INITIAL,
            .viewportHeight = HEIGHT_INITIAL,
            .window = wnd,
            .renderBackend = renderBackend
        };
        g_pEngine = std::make_unique<Engine>(config);

        gameInit(g_pEngine);

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


