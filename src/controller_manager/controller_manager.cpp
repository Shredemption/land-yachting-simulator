#include "controller_manager/controller_manager.hpp"

#include "pch.h"

void ControllerManager::update()
{
    controllerConnected = glfwJoystickIsGamepad(GLFW_JOYSTICK_1);

    if (!controllerConnected)
        return;
    else if (InputManager::inputType != InputType::Controller)
    {
        InputManager::inputType = InputType::Controller;
        glfwSetInputMode(WindowManager::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        UIManager::selected = 0;
    }

    GLFWgamepadstate newState;
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &newState))
    {
        state.update(newState);
    }

    switch (SceneManager::engineState)
    {
    case EngineState::Title:
    case EngineState::Settings:
    case EngineState::Pause:
    case EngineState::TitleSettings:
        sticksMenu();
        buttonsMenu();
        break;

    case EngineState::Running:
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
                UIManager::selected = (UIManager::selected - 1 + UIManager::options) % UIManager::options;
                break;

            case GLFW_GAMEPAD_BUTTON_BACK:
            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
                UIManager::selected = (UIManager::selected + 1 + UIManager::options) % UIManager::options;
                break;

            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
                UIManager::triggerLeft = true;
                break;

            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
                UIManager::triggerRight = true;
                break;

            case GLFW_GAMEPAD_BUTTON_A:
            case GLFW_GAMEPAD_BUTTON_START:
                UIManager::trigger = true;
                break;

            case GLFW_GAMEPAD_BUTTON_B:
                InputManager::MenuBack();
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
            UIManager::selected = (UIManager::selected - 1 + UIManager::options) % UIManager::options;
            break;

        case StickDirection::Down:
            UIManager::selected = (UIManager::selected + 1 + UIManager::options) % UIManager::options;
            break;

        case StickDirection::Left:
            UIManager::triggerLeft = true;
            break;

        case StickDirection::Right:
            UIManager::triggerRight = true;
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
                SceneManager::switchEngineState(EngineState::Pause);
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
    float x = SettingsManager::settings.input.controllerCamSensitivity * 25.0f * state.sticks[1].x;
    float y = SettingsManager::settings.input.controllerCamSensitivity * 25.0f * state.sticks[1].y;

    if (Camera::freeCam)
    {
        Camera::yawFree += glm::radians(x) * TimeManager::deltaTime;
        Camera::pitchFree += glm::radians(y) * TimeManager::deltaTime;

        Camera::pitchFree = std::clamp(Camera::pitchFree, glm::radians(-89.0f), glm::radians(89.0f));
    }
    else
    {
        Camera::yawOffset += glm::radians(x) * TimeManager::deltaTime;
        Camera::pitchOffset += glm::radians(y) * TimeManager::deltaTime;

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