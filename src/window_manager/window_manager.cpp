#include "window_manager/window_manager.hpp"

#include "pch.h"

void WindowManager::setup()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GL_TRUE);
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    // Create Window
    window = glfwCreateWindow(800, 600, "Marama", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(window);

    // Set swap interval
    glfwSwapInterval(1);

    // GLAD loads all OpenGL pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // Get screen dimensions
    glfwGetWindowPos(window, &windowXpos, &windowYpos);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
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
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;

    screenUIScale = std::min(width / 2560.0f, height / 1440.0f);

    // Track window size change for mouse movement
    windowSizeChanged = true;

    Render::resize(width, height);
}

void WindowManager::errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}
