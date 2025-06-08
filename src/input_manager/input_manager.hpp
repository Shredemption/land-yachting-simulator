#pragma once

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>

#include "input_manager/input_manager_defs.h"

namespace InputManager
{
    // Mouse state
    inline double mousePosX, mousePosY;
    inline MouseButtonState leftMouseButton, rightMouseButton;

    // Input type
    inline InputType inputType;

    void update();
    void setCallbacks();

    void keyCallbackMenu(GLFWwindow *window, int key, int scancode, int action, int mods);
    void keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mousePosCallbackMenu(GLFWwindow *window, double xPos, double yPos);
    void mouseButtonCallbackMenu(GLFWwindow *window, int button, int action, int mods);

    void mousePosCallbackRunning(GLFWwindow *window, double xPos, double yPos);

    void processInputRunning();
};