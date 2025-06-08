#include "controller_manager/controller_manager.hpp"

#include "pch.h"

void ControllerManager::update()
{
    controllerConnected = glfwJoystickIsGamepad(GLFW_JOYSTICK_1);

    if (!controllerConnected)
        return;
    else if (InputManager::inputType != InputType::itController)
    {
        InputManager::inputType = InputType::itController;
        glfwSetInputMode(WindowManager::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    GLFWgamepadstate newState;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &newState))
    {
        state.update(newState);
    }

    switch (SceneManager::engineState)
    {
    case EngineState::esTitle:
    case EngineState::esSettings:
    case EngineState::esPause:
    case EngineState::esTitleSettings:
        sticksMenu();
        buttonsMenu();
        break;

    case EngineState::esRunning:
        sticksRunning();
        buttonsRunning();
        break;
    }
}

StickDirection getStickDirection(StickInput stick)
{
    if (fabs(stick.x) > fabs(stick.y))
    {
        if (stick.x > DEADZONE)
            return StickDirection::Right;
        if (stick.x < -DEADZONE)
            return StickDirection::Left;
    }
    else
    {
        if (stick.y > DEADZONE)
            return StickDirection::Down;
        if (stick.y < -DEADZONE)
            return StickDirection::Up;
    }
    return StickDirection::None;
}

void ControllerManager::buttonsMenu()
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (state.buttons[i].pressed())
        {
            switch (i)
            {
            case GLFW_GAMEPAD_BUTTON_DPAD_UP:
                InputManager::menuMoveUp();
                break;

            case GLFW_GAMEPAD_BUTTON_BACK:
            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
                InputManager::menuMoveDown();
                break;

            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
                InputManager::menuMoveLeft();
                break;

            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
                InputManager::menuMoveRight();
                break;

            case GLFW_GAMEPAD_BUTTON_A:
            case GLFW_GAMEPAD_BUTTON_START:
                InputManager::menuRunSelected();
                break;

            case GLFW_GAMEPAD_BUTTON_B:
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
            }
        }
    }
}

void ControllerManager::sticksMenu()
{
    if (shouldTriggerNavigation(state.sticks[0]))
    {
        switch (state.sticks[0].currentDir)
        {
        case StickDirection::Up:
            InputManager::menuMoveUp();
            break;

        case StickDirection::Down:
            InputManager::menuMoveDown();
            break;

        case StickDirection::Left:
            InputManager::menuMoveLeft();
            break;

        case StickDirection::Right:
            InputManager::menuMoveRight();
            break;
        }
    }
}

void ControllerManager::buttonsRunning()
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (state.buttons[i].pressed())
        {
            switch (i)
            {
            case GLFW_GAMEPAD_BUTTON_START:
                SceneManager::switchEngineState(EngineState::esPause);
                break;

            case GLFW_GAMEPAD_BUTTON_BACK:
                PhysicsUtil::switchControlledYacht();
                break;

            case GLFW_GAMEPAD_BUTTON_Y:
                Camera::freeCam = (Camera::freeCam) ? false : true;
            }
        }
    }
}

void ControllerManager::sticksRunning()
{
    float sensitivity = 5.0f;
    float x = sensitivity / 3 * state.sticks[1].x;
    float y = sensitivity / 3 * state.sticks[1].y;

    if (Camera::freeCam)
    {
        Camera::yawFree += glm::radians(x);
        Camera::pitchFree += glm::radians(y);

        Camera::pitchFree = std::clamp(Camera::pitchFree, glm::radians(-89.0f), glm::radians(89.0f));
    }
    else
    {
        Camera::yawOffset += glm::radians(x);
        Camera::pitchOffset += glm::radians(y);

        Camera::yawOffset = std::clamp(Camera::yawOffset, glm::radians(-100.0f), glm::radians(100.0f));
        Camera::pitchOffset = std::clamp(Camera::pitchOffset, glm::radians(-45.0f), glm::radians(60.0f));
    }
}

bool ControllerManager::shouldTriggerNavigation(StickInput &stick)
{
    StickDirection newDir = getStickDirection(stick);

    if (newDir == StickDirection::None)
    {
        stick.currentDir = StickDirection::None;
        stick.holdTime = 0.0f;
        stick.repeatCooldown = 0.0f;
        return false;
    }

    if (newDir != stick.currentDir)
    {
        stick.currentDir = newDir;
        stick.holdTime = 0.0f;
        stick.repeatCooldown = INITIAL_DELAY;
        return true;
    }

    stick.holdTime += TimeManager::deltaTime;
    stick.repeatCooldown -= TimeManager::deltaTime;

    if (stick.repeatCooldown <= 0.0f)
    {
        stick.repeatCooldown = REPEAT_RATE;
        return true;
    }

    return false;
}