#include "input_manager/input_manager.hpp"

#include "pch.h"

#include "input_manager/input_manager_defs.h"
#include "ui_manager/ui_manager_defs.h"

void keyCallbackMenu(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    if (InputManager::inputType != InputType::Keyboard)
    {
        InputManager::inputType = InputType::Keyboard;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            InputManager::MenuBack();
            break;

        case GLFW_KEY_ENTER:
        case GLFW_KEY_SPACE:
            UIManager::trigger = true;
            break;

        case GLFW_KEY_W:
        case GLFW_KEY_UP:
            UIManager::selected = (UIManager::selected - 1 + UIManager::options) % UIManager::options;
            break;

        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
            UIManager::selected = (UIManager::selected + 1 + UIManager::options) % UIManager::options;
            break;

        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            UIManager::triggerRight = true;
            break;

        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
            UIManager::triggerLeft = true;
            break;
        }
    }
}

void keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            SceneManager::switchEngineState(EngineState::Pause);
            break;

        case GLFW_KEY_C:
            Camera::freeCam = !Camera::freeCam;
            break;

        case GLFW_KEY_N:
            PhysicsUtil::switchControlledYacht();
            break;
        }
    }
}

void mousePosCallbackMenu(GLFWwindow *window, double xPos, double yPos)
{
    if (ControllerManager::controllerConnected)
        return;

    if (InputManager::inputType != InputType::Mouse)
    {
        InputManager::inputType = InputType::Mouse;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        UIManager::selected = -1;
    }

    InputManager::mousePosX = xPos;
    InputManager::mousePosY = yPos;
}

void mouseButtonCallbackMenu(GLFWwindow *window, int button, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        InputManager::leftMouseButton.wasDown = InputManager::leftMouseButton.isDown;

        if (action == GLFW_PRESS)
            InputManager::leftMouseButton.isDown = true;
        else if (action == GLFW_RELEASE)
            InputManager::leftMouseButton.isDown = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        InputManager::rightMouseButton.wasDown = InputManager::rightMouseButton.isDown;

        if (action == GLFW_PRESS)
            InputManager::rightMouseButton.isDown = true;
        else if (action == GLFW_RELEASE)
            InputManager::rightMouseButton.isDown = false;
    }
}

void mousePosCallbackRunning(GLFWwindow *window, double xPos, double yPos)
{
    if (ControllerManager::controllerConnected)
        return;

    // Check if window size changed last iteration
    if (WindowManager::firstFrame || WindowManager::windowSizeChanged)
    {
        xPos = WindowManager::screenWidth / 2;
        yPos = WindowManager::screenHeight / 2;

        // Reset state changed trackers
        WindowManager::firstFrame = false;
        WindowManager::windowSizeChanged = false;

        // Reset mouse to 0,0
        glfwSetCursorPos(window, WindowManager::screenWidth / 2, WindowManager::screenHeight / 2);
    }

    xPos -= WindowManager::screenWidth / 2;
    yPos -= WindowManager::screenHeight / 2;

    // Apply sensitivity
    xPos *= SettingsManager::settings.input.mouseSensitivity / 50;
    yPos *= SettingsManager::settings.input.mouseSensitivity / 50;

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
        glfwSetCursorPos(window, WindowManager::screenWidth / 2, WindowManager::screenHeight / 2);
    }
}

void InputManager::update()
{
    leftMouseButton.wasDown = leftMouseButton.isDown;
    rightMouseButton.wasDown = rightMouseButton.isDown;
}

void InputManager::setCallbacks()
{
    GLFWwindow *window = WindowManager::window;

    switch (SceneManager::engineState)
    {
    case EngineState::Title:
        glfwSetKeyCallback(window, keyCallbackMenu);
        glfwSetCursorPosCallback(window, mousePosCallbackMenu);
        glfwSetMouseButtonCallback(window, mouseButtonCallbackMenu);
        break;

    case EngineState::Running:
        glfwSetKeyCallback(window, keyCallbackRunning);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mousePosCallbackRunning);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetCursorPos(window, WindowManager::screenWidth / 2, WindowManager::screenHeight / 2);
        break;

    case EngineState::Pause:
        glfwSetKeyCallback(window, keyCallbackMenu);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, mousePosCallbackMenu);
        glfwSetMouseButtonCallback(window, mouseButtonCallbackMenu);
        break;

    case EngineState::Settings:
    case EngineState::TitleSettings:
    case EngineState::TestMenu:
        glfwSetKeyCallback(window, keyCallbackMenu);
        glfwSetCursorPosCallback(window, mousePosCallbackMenu);
        glfwSetMouseButtonCallback(window, mouseButtonCallbackMenu);
        break;

    default:
        glfwSetKeyCallback(window, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        break;
    }

    SceneManager::updateCallbacks = false;
}

void InputManager::processInputRunning()
{
    if (ControllerManager::controllerConnected)
        return;

    GLFWwindow *window = WindowManager::window;

    // Move cam with WASD, space, shift
    float cameraSpeed = 5.f * TimeManager::deltaTime;

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

void InputManager::MenuBack()
{
    switch (SceneManager::engineState)
    {
    case EngineState::Title:
        UIManager::queueEngineState(EngineState::None);
        break;
    case EngineState::Settings:
        if (SceneManager::settingsPage != SettingsPage::Start)
        {
            UIManager::queueSettingsPage(SettingsPage::Start);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Start);
        }
        else
            UIManager::queueEngineState(EngineState::Pause);
        break;
    case EngineState::TitleSettings:
        if (SceneManager::settingsPage != SettingsPage::Start)
        {
            UIManager::queueSettingsPage(SettingsPage::Start);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Start);
        }
        else
            UIManager::queueEngineState(EngineState::Title);
        break;
    case EngineState::Pause:
        UIManager::queueEngineState(EngineState::Running);
        break;

    case EngineState::TestMenu:
        UIManager::queueEngineState(EngineState::Title);
        break;
    }
}