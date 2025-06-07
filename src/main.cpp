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

#include "event_handler/event_handler.hpp"
#include "model/model_util.hpp"
#include "scene_manager/scene_manager.hpp"
#include "scene_manager/scene_manager_defs.h"
#include "settings_manager/settings_manager.hpp"
#include "physics/physics_util.hpp"
#include "render/render.hpp"
#include "thread_manager/thread_manager.hpp"
#include "ui_manager/ui_manager.hpp"

#define STB_IMAGE_IMPLEMENTATION

int main()
{
// Attach to existing console
#ifdef _WIN32
    InitAppUserModelID();
    AttachConsoleIfNeeded();
#endif

    SettingsManager::load();

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
    EventHandler::window = glfwCreateWindow(800, 600, "Marama", nullptr, nullptr);
    if (!EventHandler::window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(EventHandler::window);

#ifdef _WIN32
    SetWindowIconFromResource(EventHandler::window);
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
    ModelUtil::loadModelMap();
    SceneManager::loadSceneMap();

    // Get screen dimensions
    glfwGetWindowPos(EventHandler::window, &EventHandler::windowXpos, &EventHandler::windowYpos);
    glfwGetWindowSize(EventHandler::window, &EventHandler::windowWidth, &EventHandler::windowHeight);
    glfwGetFramebufferSize(EventHandler::window, &EventHandler::screenWidth, &EventHandler::screenHeight);

    glfwSetFramebufferSizeCallback(EventHandler::window, EventHandler::framebufferSizeCallback);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable Depth buffer (Z-buffer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Setup render class
    Render::setup();

    // Set window to fullscreen by default
    EventHandler::setFullscreenState();

    glfwShowWindow(EventHandler::window);

    // Launch threads
    ThreadManager::startup();

    SceneManager::switchEngineState(EngineState::esTitle);

    UIManager::load(SceneManager::engineState);

    glfwPollEvents();
    glfwSwapBuffers(EventHandler::window);

    // Main Loop
    while (!glfwWindowShouldClose(EventHandler::window))
    {
        EngineState checkState = (SceneManager::exitState == EngineState::esNone) ? SceneManager::engineState : SceneManager::exitState;
        SettingsPage checkPage = (SceneManager::exitPage == SettingsPage::spNone) ? SceneManager::settingsPage : SceneManager::exitPage;

        EventHandler::timing(checkState);

        if (SceneManager::exitState == EngineState::esNone && SceneManager::updateCallbacks)
            EventHandler::setCallbacks();

        // If window inactive
        if (glfwGetWindowAttrib(EventHandler::window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }

        SceneManager::updateFade();

        switch (checkState)
        {
        case EngineState::esNone:
            Render::renderBlankScreen();
            glfwSetWindowShouldClose(EventHandler::window, GLFW_TRUE);
            break;

        case EngineState::esLoading:
            SceneManager::checkLoading();
            Render::renderLoadingScreen();
            break;

        case EngineState::esTitle:
            Render::renderTitleScreen();
            UIManager::update();
            break;

        case EngineState::esPause:
            Render::renderMenuScreen(checkState, checkPage);
            UIManager::update();
            break;

        case EngineState::esSettings:
        case EngineState::esTitleSettings:
            Render::renderMenuScreen(checkState, checkPage);
            UIManager::update();
            break;

        case EngineState::esRunning:
            EventHandler::processInputRunning();
            PhysicsUtil::update();
            Render::render();
            break;
        }

        glfwSwapBuffers(EventHandler::window);
        EventHandler::update();
        glfwPollEvents();
    }

    // Close threads
    ThreadManager::shutdown();

    // Clear scene data
    SceneManager::unload();

    // Cleanup GLFW
    glfwDestroyWindow(EventHandler::window);
    glfwTerminate();

    return 0;
}