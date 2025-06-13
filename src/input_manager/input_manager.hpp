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

    void processInputRunning();

    void menuMoveUp();
    void menuMoveDown();
    void menuMoveLeft();
    void menuMoveRight();
    void menuReturn();
    void menuRunSelected();
};