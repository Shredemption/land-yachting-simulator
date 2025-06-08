#pragma once

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <cmath>

struct ControllerButton
{
    bool isPressed = false;
    bool wasPressed = false;

    bool pressed() const { return isPressed && !wasPressed; }
    bool released() const { return !isPressed && wasPressed; }
    bool held() const { return isPressed; }
};

enum class StickDirection
{
    None,
    Up,
    Right,
    Down,
    Left
};

const float DEADZONE = 0.3f;
const float INITIAL_DELAY = 0.3f;
const float REPEAT_RATE = 0.05f;

struct StickInput
{
    float x = 0.0f;
    float y = 0.0f;

    StickDirection currentDir = StickDirection::None;
    float holdTime = 0.0f;
    float repeatCooldown = 0.0f;
};

constexpr int BUTTON_COUNT = GLFW_GAMEPAD_BUTTON_LAST + 1;

struct ControllerState
{
    ControllerButton buttons[BUTTON_COUNT];
    StickInput sticks[2];

    void update(const GLFWgamepadstate &newState)
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            buttons[i].wasPressed = buttons[i].isPressed;
            buttons[i].isPressed = (newState.buttons[i] == GLFW_PRESS);
        }

        sticks[0].x = newState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        sticks[0].y = newState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        sticks[1].x = newState.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        sticks[1].y = newState.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
    }
};