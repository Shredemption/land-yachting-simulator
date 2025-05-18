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

    // Set swap interval
    glfwSwapInterval(0);

    // GLAD loads all OpenGL pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Import JSON file model registry
    Model::loadModelMap();
    SceneManager::loadSceneMap();

    // Load title screen Scene
    SceneManager::load("title");

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

    Render::setup();

    ThreadManager::startup();

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }

        if (SceneManager::loadingState > 0)
        {
            // Check loading state
            SceneManager::checkLoading();

            // Render loading screen
            SceneManager::renderLoading();
        }

        if (SceneManager::loadingState == 0)
        {
            // Update Events
            EventHandler::update(window);

            // Update Physics
            Physics::accumulator += EventHandler::deltaTime;
            if (Physics::accumulator >= Physics::tickRate)
            {
                {
                    std::lock_guard lock(ThreadManager::physicsMutex);
                    ThreadManager::physicsTrigger = true;
                }
                ThreadManager::physicsCV.notify_one();

                Physics::accumulator -= Physics::tickRate;
            }

            // Update Animations
            {
                std::lock_guard lock(ThreadManager::animationMutex);
                ThreadManager::animationTrigger = true;
            }
            ThreadManager::animationCV.notify_one();

            // Update cam and render
            Camera::update();
            Render::render(*SceneManager::currentScene);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ThreadManager::shutdown();

    SceneManager::unload();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}