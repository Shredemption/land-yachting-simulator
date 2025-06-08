#include "input_manager/input_manager.hpp"

#include "pch.h"

#include "input_manager/input_manager_defs.h"

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
    case EngineState::esTitle:
        glfwSetKeyCallback(window, keyCallbackMenu);
        glfwSetCursorPosCallback(window, mousePosCallbackMenu);
        glfwSetMouseButtonCallback(window, mouseButtonCallbackMenu);
        break;

    case EngineState::esRunning:
        glfwSetKeyCallback(window, keyCallbackRunning);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mousePosCallbackRunning);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetCursorPos(window, WindowManager::screenWidth / 2, WindowManager::screenHeight / 2);
        break;

    case EngineState::esPause:
        glfwSetKeyCallback(window, keyCallbackMenu);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, mousePosCallbackMenu);
        glfwSetMouseButtonCallback(window, mouseButtonCallbackMenu);
        break;

    case EngineState::esSettings:
    case EngineState::esTitleSettings:
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

void InputManager::keyCallbackMenu(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    if (inputType != InputType::itKeyboard)
    {
        inputType = InputType::itKeyboard;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    // ESC handling
    if (action == GLFW_PRESS)
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            switch (SceneManager::engineState)
            {
            case EngineState::esTitle:
                SceneManager::switchEngineState(EngineState::esNone);
                break;

            case EngineState::esPause:
                SceneManager::switchEngineState(EngineState::esRunning);
                break;

            case EngineState::esSettings:
                SceneManager::switchEngineState(EngineState::esPause);
                SceneManager::switchSettingsPage(SettingsPage::spStart);
                UIManager::inputState = UIInputState::uiMain;
                break;

            case EngineState::esTitleSettings:
                SceneManager::switchEngineState(EngineState::esTitle);
                SceneManager::switchSettingsPage(SettingsPage::spStart);
                UIManager::inputState = UIInputState::uiMain;
                break;
            }
            break;

        case GLFW_KEY_W:
        case GLFW_KEY_UP:
            menuMoveUp();
            break;

        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
            menuMoveDown();
            break;

        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
            menuMoveLeft();
            break;

        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            menuMoveRight();
            break;

        case GLFW_KEY_ENTER:
        case GLFW_KEY_SPACE:
            menuRunSelected();
            break;
        }
}

void InputManager::keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    // Pause on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        SceneManager::switchEngineState(EngineState::esPause);

    // Toggle FPS debug on F9
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS)
        Render::debugstate = (Render::debugstate == debugState::dbFPS) ? debugState::dbNone : debugState::dbFPS;

    // Toggle physics debug on F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS)
        Render::debugstate = (Render::debugstate == debugState::dbPhysics) ? debugState::dbNone : debugState::dbPhysics;

    // Toggle Freecam on C
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
        Camera::freeCam = (Camera::freeCam) ? false : true;

    // Switch to next controllable yacht on N
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        PhysicsUtil::switchControlledYacht();
}

void InputManager::mousePosCallbackMenu(GLFWwindow *window, double xPos, double yPos)
{
    if (ControllerManager::controllerConnected)
        return;

    if (inputType != InputType::itMouse)
    {
        inputType = InputType::itMouse;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    mousePosX = xPos;
    mousePosY = yPos;
}

void InputManager::mouseButtonCallbackMenu(GLFWwindow *window, int button, int action, int mods)
{
    if (ControllerManager::controllerConnected)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        leftMouseButton.wasDown = leftMouseButton.isDown;

        if (action == GLFW_PRESS)
            leftMouseButton.isDown = true;
        else if (action == GLFW_RELEASE)
            leftMouseButton.isDown = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        rightMouseButton.wasDown = rightMouseButton.isDown;

        if (action == GLFW_PRESS)
            rightMouseButton.isDown = true;
        else if (action == GLFW_RELEASE)
            rightMouseButton.isDown = false;
    }
}

void InputManager::mousePosCallbackRunning(GLFWwindow *window, double xPos, double yPos)
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
    float sensitvity = 5.0f;
    xPos *= sensitvity / 50;
    yPos *= sensitvity / 50;

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

void InputManager::menuMoveUp()
{
    switch (UIManager::inputState)
    {
    case UIInputState::uiMain:
        UIManager::selectedMain = (UIManager::selectedMain <= 0) ? UIManager::uiElements.size() - 1 : UIManager::selectedMain - 1;
        break;

    case UIInputState::uiSide:
        UIManager::selectedSide = (UIManager::selectedSide <= 0) ? UIManager::uiElementsSide.size() - 1 : UIManager::selectedSide - 1;
        break;
    }
}

void InputManager::menuMoveDown()
{
    switch (UIManager::inputState)
    {
    case UIInputState::uiMain:
        UIManager::selectedMain = (UIManager::selectedMain == UIManager::uiElements.size() - 1) ? 0 : UIManager::selectedMain + 1;
        break;

    case UIInputState::uiSide:
        UIManager::selectedSide = (UIManager::selectedSide == UIManager::uiElementsSide.size() - 1) ? 0 : UIManager::selectedSide + 1;
        break;
    }
}

void InputManager::menuMoveLeft()
{
    if (UIManager::inputState == UIInputState::uiSide)
        if (SceneManager::settingsPage != SettingsPage::spStart)
        {
            UIManager::inputState = UIInputState::uiMain;
        }
}

void InputManager::menuMoveRight()
{
    if (UIManager::inputState == UIInputState::uiMain)
        if (SceneManager::settingsPage != SettingsPage::spStart)
        {
            UIManager::inputState = UIInputState::uiSide;
        }
}

void InputManager::menuRunSelected()
{
    auto *selected = UIManager::getSelectedElement();
    if (!selected)
        return;

    std::visit([](auto *element)
               {
                if (!element)
                    return;
                    
                    using T = std::decay_t<decltype(*element)>;
                    if constexpr (std::is_same_v<T, UIButton>)
                        element->onClick();
                    else if constexpr (std::is_same_v<T, UIToggle>)
                        element->execute(); },
               *selected);
}
