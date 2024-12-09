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

#include "file_manager/file_manager.h"
#include "scene/scene.h"
#include "event_handler/event_handler.h"
#include "render/render.h"

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
    glfwSwapInterval(1);

    // GLAD loads all OpenGL pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Import JSON file model registry
    Model::modelMap = Model::loadModelMap("resources/models.json");

    // Import JSON file scene
    Scene scene = Scene("resources/scenes/testing.json");

    // Draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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

    // Set window to fullscreen by default
    if (EventHandler::fullscreen)
    {
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);
    }

    glfwShowWindow(window);

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            glfwWaitEvents();
            continue;
        }
        else
        {
            // Get time since launch
            EventHandler::time = (float)glfwGetTime();
            EventHandler::deltaTime = EventHandler::time - EventHandler::lastTime;
            EventHandler::lastTime = EventHandler::time;

            // Clear color buffer
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Process Inputs
            EventHandler::processInput(window);

            // Projection Matrix
            Render::u_projection = glm::perspective((float)M_PI_2,                                                        // Field of view (90 deg)
                                                    (float)EventHandler::screenWidth / (float)EventHandler::screenHeight, // Aspect Ratio (w/h)
                                                    0.01f,                                                                // Near clipping plane
                                                    100.0f                                                                // Far clipping plane
            );

            // View matrix
            Render::u_view = glm::lookAt(EventHandler::cameraPosition,                                     // Camera Position
                                         EventHandler::cameraPosition + EventHandler::cameraViewDirection, // Target Position
                                         EventHandler::cameraUp                                            // Up vector
            );

            // Sun/moon lighting
            EventHandler::sunAngle += EventHandler::deltaTime * 10.0f;

            // lightColor = glm::vec3((1.5f + std::sin(glm::radians(sunAngle)) + std::sin(glm::radians(2.f * sunAngle - 90.f))) / 3.5f);
            EventHandler::lightPos = 100.0f * glm::vec3(std::sin(glm::radians(EventHandler::sunAngle)), 1.f, std::cos(glm::radians(EventHandler::sunAngle)));

            // Draw scene, using view and projection matrix for entire scene
            Render::render(scene);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // Unload Scene before stopping
    scene.~Scene();

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}