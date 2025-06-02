#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#ifndef __glad_h_
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <chrono>

class EventHandler
{
public:
    // Global screen variables
    static int xPos, yPos, screenWidth, screenHeight;
    static bool fullscreen, windowSizeChanged, firstFrame;
    static GLFWmonitor *monitor;
    static int windowXpos, windowYpos, windowWidth, windowHeight;

    // Global Time
    static double time;
    static double deltaTime;
    static std::chrono::steady_clock::time_point lastTime;
    static std::chrono::steady_clock::time_point now;
    static unsigned int frame;

    // Global Light properties
    static glm::vec3 lightPos;
    static float sunAngle;
    static float sunSpeed;
    static glm::vec3 lightCol;
    static float lightInsensity;

    // Global input/callback Functions
    static void timing(GLFWwindow *window);
    static void setCallbacks(GLFWwindow *window);
    static void errorCallback(int error, const char *description);
    static void keyCallbackTitle(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void keyCallbackRunning(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseCallbackRunning(GLFWwindow *window, double xPos, double yPos);
    static void processInputRunning(GLFWwindow *window);
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif
