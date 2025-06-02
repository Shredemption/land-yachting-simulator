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

inline void atomicAdd(std::atomic<double> &atomicVal, double value)
{
    double current = atomicVal.load(std::memory_order_relaxed);
    double desired;
    do
    {
        desired = current + value;
    } while (!atomicVal.compare_exchange_weak(current, desired, std::memory_order_release, std::memory_order_relaxed));
}

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
    glfwSetKeyCallback(window, EventHandler::keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, EventHandler::mouseCallback);
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

    // Set window to fullscreen by default
    if (EventHandler::fullscreen)
    {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    glfwShowWindow(window);

    // Setup render class
    Render::setup();

    // Launch threads
    ThreadManager::startup();

    // Load title screen Scene
    SceneManager::load("title");

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        EventHandler::timing(window);

        // If window inactive
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }

        // If on loading screen
        if (SceneManager::loadingState > 0)
        {
            // Check loading state
            SceneManager::checkLoading(window);

            // Render loading screen
            SceneManager::renderLoading();
        }

        // If normally running
        if (SceneManager::loadingState == 0)
        {
            // Update Events
            EventHandler::processInput(window);

            // Update Physics
            atomicAdd(Physics::accumulator, EventHandler::deltaTime);

            // If time for physics tick
            int steps = 0;
            double acc = Physics::accumulator.load(std::memory_order_acquire);

            while (acc >= Physics::tickRate)
            {
                acc -= Physics::tickRate;
                steps++;
            }

            if (steps > 0 && !ThreadManager::physicsBusy.load(std::memory_order_acquire))
            {
                ThreadManager::physicsBusy.store(true, std::memory_order_release);
                // Update physics
                {
                    std::lock_guard lock(ThreadManager::physicsMutex);
                    ThreadManager::physicsTrigger = true;
                    ThreadManager::physicsSteps += steps;
                }
                ThreadManager::physicsCV.notify_one();
            }

            float alpha = static_cast<float>(acc / Physics::tickRate);
            ThreadManager::animationAlpha.store(alpha, std::memory_order_release);

            // Update Animations
            {
                std::lock_guard lock(ThreadManager::animationMutex);
                ThreadManager::animationTrigger = true;
            }
            ThreadManager::animationCV.notify_one();

            // Then render the frame
            int currentIndex = Render::renderIndex.load(std::memory_order_acquire);
            auto &buffer = Render::renderBuffers[currentIndex];

            // Only render if the buffer is ready
            BufferState expected = BufferState::Ready;
            if (buffer.state.compare_exchange_strong(expected, BufferState::Rendering))
            {
                // Perform actual rendering
                Render::executeRender(buffer);

                // After rendering is done, mark buffer free
                buffer.state.store(BufferState::Free, std::memory_order_release);

                // Rotate render index to the next ready buffer (if available)
                for (int i = 0; i < 3; ++i)
                {
                    if (Render::renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Ready)
                    {
                        Render::renderIndex.store(i, std::memory_order_release);
                        break;
                    }
                }

                ThreadManager::renderBufferCV.notify_all();
            }
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