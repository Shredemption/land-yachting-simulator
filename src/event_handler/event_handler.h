#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class EventHandler
{
public:
    // Global screen variables
    static int xPos, yPos, screenWidth, screenHeight;
    static bool fullscreen, windowSizeChanged, firstFrame;
    static GLFWmonitor *monitor;
    static int windowXpos, windowYpos, windowWidth, windowHeight;

    // Global Time
    static float deltaTime;
    static float lastTime;
    static float time;
    static unsigned int frame;

    // Global Light properties
    static glm::vec3 lightPos;
    static float sunAngle;
    static float sunSpeed;
    static glm::vec3 lightCol;
    static float lightInsensity;

    // Global input/callback Functions
    static void errorCallback(int error, const char *description);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow *window, double xPos, double yPos);
    static void processInput(GLFWwindow *window);
    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};

#endif // EVENT_HANDLER_H
