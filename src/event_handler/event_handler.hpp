#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>

#include <chrono>
#include <optional>

#include "event_handler_defs.h"

enum class EngineState;

namespace EventHandler
{
    // Global screen variables
    inline int screenWidth, screenHeight;
    inline bool windowSizeChanged = false, firstFrame = true;
    inline float screenUIScale;
    inline GLFWmonitor *monitor;
    inline GLFWwindow *window;
    inline int windowXpos, windowYpos, windowWidth, windowHeight;

    // Global Time
    inline double time = 0.0;
    inline double deltaTime = 0.0;
    inline std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    inline std::chrono::steady_clock::time_point lastTime = startTime;
    inline std::chrono::steady_clock::time_point now = startTime;
    inline unsigned int frame = 0;

    inline std::optional<std::chrono::steady_clock::time_point> pauseStart{};
    inline std::chrono::duration<double> pausedDuration{0};

    // Global Light properties
    inline glm::vec3 lightPos(1000.0f, -1000.0f, 2000.0f);
    inline glm::vec3 lightCol(1.0f, 1.0f, 1.0f);
    inline float lightInsensity = 2;

    // Mouse state
    inline double mousePosX, mousePosY;
    inline MouseButtonState leftMouseButton, rightMouseButton;

    // Input type
    inline InputType inputType;

    // Global input/callback Functions
    void timing(EngineState &state);
    void update();
    void setCallbacks();

    void keyCallbackMenu(GLFWwindow *window, int key, int scancode, int action, int mods);
    void keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mousePosCallbackMenu(GLFWwindow *window, double xPos, double yPos);
    void mouseButtonCallbackMenu(GLFWwindow *window, int button, int action, int mods);

    void mousePosCallbackRunning(GLFWwindow *window, double xPos, double yPos);

    void processInputRunning();

    void errorCallback(int error, const char *description);
    void framebufferSizeCallback(GLFWwindow *window, int width, int height);

    void setFullscreenState();
};

#endif
