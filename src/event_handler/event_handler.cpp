#include "event_handler/event_handler.h"

#include <iostream>
#include <camera/camera.h>
#include <render/render.h>

#include "physics/physics.h"
#include "scene_manager/scene_manager.h"

// Global screen variables
int EventHandler::xPos, EventHandler::yPos, EventHandler::screenWidth, EventHandler::screenHeight;
int EventHandler::windowXpos, EventHandler::windowYpos, EventHandler::windowWidth, EventHandler::windowHeight;

bool EventHandler::fullscreen = true;
bool EventHandler::windowSizeChanged = false;
bool EventHandler::firstFrame = true;
GLFWmonitor *EventHandler::monitor;

// Global Time
double EventHandler::time = 0;
double EventHandler::deltaTime = 0;
std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point EventHandler::now = startTime;
std::chrono::steady_clock::time_point EventHandler::lastTime = startTime;
unsigned int EventHandler::frame = 0;

// Global Light properties
glm::vec3 EventHandler::lightPos(1000.f, -1000.f, 2000.f);
glm::vec3 EventHandler::lightCol(1, 1, 1);
float EventHandler::lightInsensity = 2;

// Generic EventHandler time update
void EventHandler::timing(GLFWwindow *window)
{
    now = std::chrono::steady_clock::now();
    std::chrono::duration<double> delta = now - lastTime;
    std::chrono::duration<double> total = now - startTime;
    deltaTime = delta.count();
    time = total.count();
    lastTime = now;
    frame++;
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
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Load scenes
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        SceneManager::loadAsync("realistic");
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        SceneManager::loadAsync("cartoon");
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        SceneManager::loadAsync("test");
    }

    keyCallbackGlobal(window, key, scancode, action, mods);
}

void EventHandler::keyCallbackPause(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Unpause on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        SceneManager::engineState = EngineState::Running;
        SceneManager::updateCallbacks = true;
    }

    // Quit on Q
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        SceneManager::load("title");
    }

    keyCallbackGlobal(window, key, scancode, action, mods);
}

void EventHandler::keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Pause on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        SceneManager::engineState = EngineState::Pause;
        SceneManager::updateCallbacks = true;
    }

    // Toggle FPS debug on F9
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
    {
        if (Render::debugState == dbFPS)
        {
            Render::debugState = dbNone;
        }
        else
        {
            Render::debugState = dbFPS;
        }
    }

    // Toggle physics debug on F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
    {
        if (Render::debugState == dbPhysics)
        {
            Render::debugState = dbNone;
        }
        else
        {
            Render::debugState = dbPhysics;
        }
    }

    // Toggle Freecam on C
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        if (Camera::freeCam)
        {
            Camera::freeCam = false;
        }
        else
        {
            Camera::freeCam = true;
        }
    }

    // Reset physics on R
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        Physics::resetState = true;
    }

    // Switch to next controllable yacht on N
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        Physics::switchControlledYacht(*SceneManager::currentScene);
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
    for (auto &key : Physics::keyInputs)
    {
        key = false;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        Physics::keyInputs[0] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        Physics::keyInputs[1] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        Physics::keyInputs[2] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        Physics::keyInputs[3] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        Physics::keyInputs[4] = true;
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