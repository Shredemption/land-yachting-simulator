#include "window_manager/window_manager.hpp"

#include "pch.h"

void WindowManager::setup(renderEngine engine)
{
    currentEngine = engine;
    std::cout << "[WindowManager] Setting up window (engine: " << (engine == renderEngine::Vulkan ? "Vulkan" : "OpenGL") << ")" << std::endl;

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "[WindowManager] ERROR: Failed to initialize GLFW" << std::endl;
        return;
    }

    std::cout << "[WindowManager]   GLFW initialized" << std::endl;

    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    if (engine == renderEngine::Vulkan)
    {
        // No OpenGL context for Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    else
    {
        // OpenGL setup (current code)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    // Create Window
    window = glfwCreateWindow(800, 600, "Marama", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "[WindowManager] ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    std::cout << "[WindowManager]   Window created (800x600)" << std::endl;

    if (engine == renderEngine::OpenGL)
    {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "[WindowManager] ERROR: Failed to initialize GLAD" << std::endl;
            return;
        }
        std::cout << "[WindowManager]   GLAD loaded" << std::endl;
    }

    // Get screen dimensions
    glfwGetWindowPos(window, &windowXpos, &windowYpos);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    std::cout << "[WindowManager] Window setup completed" << std::endl;
}

void WindowManager::setFullscreenState()
{
    bool currentlyFullscreen = glfwGetWindowAttrib(window, GLFW_DECORATED) == GLFW_FALSE;
    bool stillHidden = glfwGetWindowAttrib(window, GLFW_VISIBLE) == GLFW_FALSE;

    if (SettingsManager::settings.video.fullscreen)
    {
        if (currentlyFullscreen && !stillHidden)
            return;

        // Store old window size etc.
        glfwGetWindowPos(window, &windowXpos, &windowYpos);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Set to borderless window
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);

        windowSizeChanged = true;
    }
    else
    {
        if (!currentlyFullscreen && !stillHidden)
            return;

        // Set back to window, using saved old size etc.
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
        glfwSetWindowMonitor(window, NULL, windowXpos, windowYpos, windowWidth, windowHeight, GLFW_DONT_CARE);

        windowSizeChanged = true;
    }

    if (stillHidden)
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        framebufferSizeCallback(window, width, height);
    }
}

void WindowManager::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    if (WindowManager::currentEngine == renderEngine::OpenGL)
    {
        glViewport(0, 0, width, height);
    }

    screenWidth = width;
    screenHeight = height;

    screenUIScale = std::min(width / 2560.0f, height / 1440.0f);

    // Track window size change for mouse movement
    windowSizeChanged = true;

    pendingResizeWidth = width;
    pendingResizeHeight = height;
    resizePending = true;
}

void WindowManager::errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}

void WindowManager::swapBuffers()
{
    if (currentEngine == renderEngine::OpenGL)
    {
        glfwSwapBuffers(window);
    }
}