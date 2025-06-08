#pragma once

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

namespace WindowManager
{
    // Global screen variables
    inline int screenWidth, screenHeight;
    inline bool windowSizeChanged = false, firstFrame = true;
    inline float screenUIScale;
    inline GLFWmonitor *monitor;
    inline GLFWwindow *window;
    inline int windowXpos, windowYpos, windowWidth, windowHeight;

    void setup();
    void errorCallback(int error, const char *description);
    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    void setFullscreenState();
};