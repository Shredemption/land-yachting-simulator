#include "event_handler/event_handler.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include "camera/camera.hpp"
#include "physics/physics_util.hpp"
#include "render/render.hpp"
#include "scene_manager/scene_manager.hpp"
#include "scene_manager/scene_manager_defs.h"

// Generic EventHandler time update
void EventHandler::timing(GLFWwindow *window, EngineState &state)
{
    now = std::chrono::steady_clock::now();

    std::chrono::duration<double> delta = now - lastTime;
    deltaTime = delta.count();

    lastTime = now;
    frame++;

    bool timeShouldPause = (state == EngineState::Pause || state == EngineState::Loading);

    if (timeShouldPause)
    {
        if (!pauseStart.has_value())
            pauseStart = now;
    }
    else
    {
        if (pauseStart.has_value())
        {
            pausedDuration += now - *pauseStart;
            pauseStart.reset();
        }

        std::chrono::duration<double> total = now - startTime - pausedDuration;
        time = total.count();
    }
}

void EventHandler::setCallbacks(GLFWwindow *window)
{
    switch (SceneManager::engineState)
    {
    case EngineState::Title:
        glfwSetKeyCallback(window, EventHandler::keyCallbackTitle);
        glfwSetCursorPosCallback(window, nullptr);
        break;

    case EngineState::Running:
        glfwSetKeyCallback(window, EventHandler::keyCallbackRunning);
        glfwSetCursorPosCallback(window, EventHandler::mouseCallbackRunning);
        break;

    case EngineState::Pause:
        glfwSetKeyCallback(window, EventHandler::keyCallbackPause);
        glfwSetCursorPosCallback(window, nullptr);
        break;

    default:
        glfwSetKeyCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        break;
    }

    SceneManager::updateCallbacks = false;
}

void EventHandler::keyCallbackGlobal(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Toggle fullscreen on F12
    if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
        if (fullscreen)
        {
            // Set back to window, using saved old size etc.
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
            glfwSetWindowMonitor(window, NULL, windowXpos, windowYpos, windowWidth, windowHeight, GLFW_DONT_CARE);

            fullscreen = !fullscreen;
            windowSizeChanged = true;
        }
        else
        {
            // Store old window size etc.
            glfwGetWindowPos(window, &windowXpos, &windowYpos);
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            // Set to borderless window
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
            glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
            const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);

            fullscreen = !fullscreen;
            windowSizeChanged = true;
        }
    }
}

void EventHandler::keyCallbackTitle(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Close on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        SceneManager::switchEngineState(EngineState::None);
    }

    // Load scenes
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        SceneManager::switchEngineStateScene("realistic");
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        SceneManager::switchEngineStateScene("cartoon");
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        SceneManager::switchEngineStateScene("test");
    }

    keyCallbackGlobal(window, key, scancode, action, mods);
}

void EventHandler::keyCallbackPause(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Unpause on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        SceneManager::switchEngineState(EngineState::Running);
    }

    // To menu on Q
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        SceneManager::switchEngineState(EngineState::Title);
    }

    keyCallbackGlobal(window, key, scancode, action, mods);
}

void EventHandler::keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Pause on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        SceneManager::switchEngineState(EngineState::Pause);
    }

    // Toggle FPS debug on F9
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
    {
        Render::debugstate = (Render::debugstate == debugState::dbFPS) ? debugState::dbNone : debugState::dbFPS;
    }

    // Toggle physics debug on F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
    {
        Render::debugstate = (Render::debugstate == debugState::dbPhysics) ? debugState::dbNone : debugState::dbPhysics;
    }

    // Toggle Freecam on C
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        Camera::freeCam = (Camera::freeCam) ? false : true;
    }

    // Switch to next controllable yacht on N
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        PhysicsUtil::switchControlledYacht(*SceneManager::currentScene);
    }

    keyCallbackGlobal(window, key, scancode, action, mods);
}

void EventHandler::mouseCallbackRunning(GLFWwindow *window, double xPos, double yPos)
{
    // Check if window size changed last iteration
    if (firstFrame || windowSizeChanged)
    {
        xPos = 0;
        yPos = 0;

        // Reset state changed trackers
        firstFrame = false;
        windowSizeChanged = false;

        // Reset mouse to 0,0
        glfwSetCursorPos(window, 0, 0);
    }

    // Apply sensitivity
    float sensitvity = 0.1f;
    xPos *= sensitvity;
    yPos *= sensitvity;

    // If camera moved
    if (xPos != 0 || yPos != 0)
    {
        Camera::cameraMoved = true;

        // Update yaw and pitch
        if (Camera::freeCam)
        {
            // Convert to radians
            Camera::yawFree += glm::radians(xPos);
            Camera::pitchFree += glm::radians(yPos);

            // Apply limits
            Camera::pitchFree = std::clamp(Camera::pitchFree, glm::radians(-89.0f), glm::radians(89.0f));
        }
        else
        {
            // Convert to radians
            Camera::yawOffset += glm::radians(xPos);
            Camera::pitchOffset += glm::radians(yPos);

            // Apply limits
            Camera::yawOffset = std::clamp(Camera::yawOffset, glm::radians(-100.0f), glm::radians(100.0f));
            Camera::pitchOffset = std::clamp(Camera::pitchOffset, glm::radians(-45.0f), glm::radians(60.0f));
        }

        // Reset mouse to 0,0
        glfwSetCursorPos(window, 0, 0);
    }
}

void EventHandler::processInputRunning(GLFWwindow *window)
{
    // Move cam with WASD, space, shift
    float cameraSpeed = 5.f * deltaTime;

    // Find XY plane view direction
    glm::vec3 forwardXY = Camera::cameraViewDirection;
    forwardXY.z = 0.f;
    forwardXY = glm::normalize(forwardXY);

    // Apply camera movement per button pressed
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree += cameraSpeed * forwardXY;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree -= cameraSpeed * forwardXY;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree -= glm::normalize(glm::cross(forwardXY, Camera::worldUp)) * cameraSpeed;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree += glm::normalize(glm::cross(forwardXY, Camera::worldUp)) * cameraSpeed;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree += cameraSpeed * Camera::worldUp;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && Camera::freeCam)
    {
        Camera::cameraPositionFree -= cameraSpeed * Camera::worldUp;
        Camera::cameraMoved = true;
    }

    // Physics Keys
    // Set all keyspresses to false
    for (auto &key : PhysicsUtil::keyInputs)
    {
        key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        PhysicsUtil::keyInputs[0] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        PhysicsUtil::keyInputs[1] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        PhysicsUtil::keyInputs[2] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        PhysicsUtil::keyInputs[3] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        PhysicsUtil::keyInputs[4] = true;
    }
}

void EventHandler::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;

    // Track window size change for mouse movement
    windowSizeChanged = true;

    Render::resize(width, height);
}

void EventHandler::errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}