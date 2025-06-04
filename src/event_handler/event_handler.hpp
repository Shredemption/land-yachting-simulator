#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>

#include <chrono>
#include <optional>

enum class EngineState;

namespace EventHandler
{
    // Global screen variables
    inline int xPos, yPos, screenWidth, screenHeight;
    inline bool fullscreen = true, windowSizeChanged = false, firstFrame = true;
    inline GLFWmonitor *monitor;
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

    // Global input/callback Functions
    void timing(GLFWwindow *window, EngineState &state);
    void setCallbacks(GLFWwindow *window);

    void keyCallbackGlobal(GLFWwindow *window, int key, int scancode, int action, int mods);

    void keyCallbackTitle(GLFWwindow *window, int key, int scancode, int action, int mods);

    void keyCallbackPause(GLFWwindow *window, int key, int scancode, int action, int mods);

    void keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mouseCallbackRunning(GLFWwindow *window, double xPos, double yPos);
    void processInputRunning(GLFWwindow *window);

    void errorCallback(int error, const char *description);
    void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif
