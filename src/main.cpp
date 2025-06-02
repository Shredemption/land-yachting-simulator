#ifdef _WIN32
#include <windows.h>
#include <iostream>

void AttachConsoleIfNeeded()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        FILE *dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        std::cout.clear();
        std::cerr.clear();
    }
}
#endif

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <shellapi.h>
#include <shobjidl.h>
#include <GLFW/glfw3native.h> // for glfwGetWin32Window()

void InitAppUserModelID()
{
    // e.g. "com.yourcompany.yourapp"
    const wchar_t *AppID = L"Marama.YourCompany.MaramaApp";
    SetCurrentProcessExplicitAppUserModelID(AppID);
}

void SetWindowIconFromResource(GLFWwindow *window)
{
    // Get the HWND for the GLFW window
    HWND hwnd = glfwGetWin32Window(window);
    // Load the icon you embedded (make sure IDI_ICON1 matches your .rc)
    HICON hIconSmall = (HICON)LoadImage(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(101),
        IMAGE_ICON,
        16, 16,
        LR_DEFAULTCOLOR);
    HICON hIconBig = (HICON)LoadImage(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(101),
        IMAGE_ICON,
        32, 32,
        LR_DEFAULTCOLOR);
    // Send both small and large
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);

    SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)hIconBig);
    SetClassLongPtr(hwnd, GCLP_HICONSM, (LONG_PTR)hIconSmall);
}
#endif

#include "event_handler/event_handler.h"
#include "model/model.h"
#include "scene/scene.h"
#include "scene_manager/scene_manager.h"
#include "camera/camera.h"
#include "render/render.h"
#include "physics/physics.h"
#include "thread_manager/thread_manager.h"

#define STB_IMAGE_IMPLEMENTATION

int main()
{
// Attach to existing console
#ifdef _WIN32
    InitAppUserModelID();
    AttachConsoleIfNeeded();
#endif

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW error callback
    glfwSetErrorCallback(EventHandler::errorCallback);

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    // Create Window
    GLFWwindow *window = glfwCreateWindow(800, 600, "Marama", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(window);

#ifdef _WIN32
    SetWindowIconFromResource(window);
#endif

    // Set swap interval
    glfwSwapInterval(1);

    // GLAD loads all OpenGL pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Import JSON file model registry
    Model::loadModelMap();
    SceneManager::loadSceneMap();

    // Get screen dimensions
    glfwGetWindowPos(window, &EventHandler::windowXpos, &EventHandler::windowYpos);
    glfwGetWindowSize(window, &EventHandler::windowWidth, &EventHandler::windowHeight);
    glfwGetFramebufferSize(window, &EventHandler::screenWidth, &EventHandler::screenHeight);

    // Set Keycallback for window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, EventHandler::framebufferSizeCallback);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable Depth buffer (Z-buffer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Wireframe Mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Setup render class
    Render::setup();

    // Set window to fullscreen by default
    if (EventHandler::fullscreen)
    {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    glfwShowWindow(window);

    // Launch threads
    ThreadManager::startup();

    // Load title screen Scene
    SceneManager::load("title");

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        EventHandler::timing(window);

        if (SceneManager::updateCallbacks)
            EventHandler::setCallbacks(window);

        // If window inactive
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }

        // Run accoring to state
        switch (SceneManager::engineState)
        {
        case EngineState::Loading:
            SceneManager::checkLoading(window);
            Render::renderLoading();
            break;

        case EngineState::Title:
            Render::render();
            break;

        case EngineState::Pause:
            Render::renderPauseScreen();
            break;

        case EngineState::Running:
            EventHandler::processInputRunning(window);
            Physics::update();
            Render::render();
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Close threads
    ThreadManager::shutdown();

    // Clear scene data
    SceneManager::unload();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}