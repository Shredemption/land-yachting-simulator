#include "pch.h"

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

int main()
{
// Attach to existing console
#ifdef _WIN32
    InitAppUserModelID();
    AttachConsoleIfNeeded();
#endif

    SettingsManager::load();

    WindowManager::setup();

#ifdef _WIN32
    SetWindowIconFromResource(WindowManager::window);
#endif

    // Import JSON file model registry
    ModelUtil::loadModelMap();
    SceneManager::loadSceneMap();

    Render::setup();

    // Set window to fullscreen by default
    WindowManager::setFullscreenState();

    glfwShowWindow(WindowManager::window);

    // Launch threads
    ThreadManager::startup();

    SceneManager::switchEngineState(EngineState::Title);

    InputManager::setCallbacks();

    glfwPollEvents();
    glfwSwapBuffers(WindowManager::window);

    // Main Loop
    while (!glfwWindowShouldClose(WindowManager::window))
    {
        TimeManager::timing(SceneManager::engineState);

        if (SceneManager::updateCallbacks)
            InputManager::setCallbacks();

        // If window inactive
        if (glfwGetWindowAttrib(WindowManager::window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }

        switch (SceneManager::engineState)
        {
        case EngineState::None:
            Render::renderBlankScreen();
            glfwSetWindowShouldClose(WindowManager::window, GLFW_TRUE);
            break;

        case EngineState::Loading:
            SceneManager::checkLoading();
            Render::renderLoadingScreen();
            break;

        case EngineState::Title:
        case EngineState::Pause:
        case EngineState::Settings:
        case EngineState::TitleSettings:
            UIManager::update();
            Render::renderMenu(SceneManager::engineState);
            break;

        case EngineState::Running:
            InputManager::processInputRunning();
            PhysicsUtil::update();
            Render::render();
            break;
        }

        glfwSwapBuffers(WindowManager::window);
        InputManager::update();
        ControllerManager::update();
        glfwPollEvents();
    }

    // Close threads
    ThreadManager::shutdown();

    // Clear scene data
    SceneManager::unload();

    // Cleanup GLFW
    glfwDestroyWindow(WindowManager::window);
    glfwTerminate();

    return 0;
}