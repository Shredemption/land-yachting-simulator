#include <event_handler/event_handler.h>

#include <iostream>
#include <camera/camera.h>

// Global screen variables
int EventHandler::xPos, EventHandler::yPos, EventHandler::screenWidth, EventHandler::screenHeight;
bool EventHandler::fullscreen = true;
bool EventHandler::windowSizeChanged = false;
bool EventHandler::firstFrame = true;
GLFWmonitor *EventHandler::monitor;
int EventHandler::windowXpos, EventHandler::windowYpos, EventHandler::windowWidth, EventHandler::windowHeight;

// Global Time
float EventHandler::time;
float EventHandler::deltaTime = 0;
float EventHandler::lastTime = 0;
unsigned int EventHandler::frame = 0;

// Global Light properties
glm::vec3 EventHandler::lightPos(0.f, 3.f, 10.f);
float EventHandler::sunAngle = rand() % 360;
float EventHandler::sunSpeed = 1.0f;
glm::vec3 EventHandler::lightCol(1, 1, 1);
float EventHandler::lightInsensity = 2;

// Keycallback to define buttom presses
void EventHandler::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Close on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Toggle fullscreen on
    if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
    {
        if (fullscreen)
        {
            // Set back to window, using saved old size etc.
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
            glfwSetWindowMonitor(window, NULL, windowXpos, windowYpos, windowWidth, windowHeight, GLFW_DONT_CARE);

            fullscreen = !fullscreen;
        }
        else
        {
            // Store old window size etc.
            glfwGetWindowPos(window, &windowXpos, &windowYpos);
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            // Set to borderless window
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
            glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
            const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);

            fullscreen = !fullscreen;
        }
    }
}

void EventHandler::mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    // Check if window size changed last iteration
    if (firstFrame)
    {
        xPos = 0;
        yPos = 0;
        firstFrame = false;

        // Reset mouse to 0,0
        glfwSetCursorPos(window, 0, 0);
    }

    if (windowSizeChanged)
    {
        xPos = 0;
        yPos = 0;
        windowSizeChanged = false;
    }

    // Apply sensitivity
    float sensitvity = 0.1f;
    xPos *= sensitvity;
    yPos *= sensitvity;

    if (xPos != 0 || yPos != 0)
    {
        Camera::cameraMoved = true;

        // Update yaw and pitch
        Camera::yaw += glm::radians(xPos); // Convert to radians
        Camera::pitch += glm::radians(yPos);
        // roll += 0;

        // Clamp the pitch to prevent flipping
        if (Camera::pitch > glm::radians(85.0f)) // Maximum upward angle
            Camera::pitch = glm::radians(85.0f);
        if (Camera::pitch < glm::radians(-85.0f)) // Maximum downward angle
            Camera::pitch = glm::radians(-85.0f);

        // Generate new direction vector(s)
        Camera::setCamDirection();

        // Reset mouse to 0,0
        glfwSetCursorPos(window, 0, 0);
    }
}

void EventHandler::processInput(GLFWwindow *window)
{
    // Move cam with WASD, space, shift
    float cameraSpeed = 5.f * deltaTime;

    // Find XY plane view direction
    glm::vec3 forwardXY = Camera::cameraViewDirection;
    forwardXY.z = 0.f;
    forwardXY = glm::normalize(forwardXY);

    // Apply correct movement per button pressed
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Camera::cameraPosition += cameraSpeed * forwardXY;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        Camera::cameraPosition -= cameraSpeed * forwardXY;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        Camera::cameraPosition -= glm::normalize(glm::cross(forwardXY, Camera::worldUp)) * cameraSpeed;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        Camera::cameraPosition += glm::normalize(glm::cross(forwardXY, Camera::worldUp)) * cameraSpeed;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        Camera::cameraPosition += cameraSpeed * Camera::worldUp;
        Camera::cameraMoved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        Camera::cameraPosition -= cameraSpeed * Camera::worldUp;
        Camera::cameraMoved = true;
    }
}

void EventHandler::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;

    // Track window size change for mouse movement
    windowSizeChanged = true;
}

void EventHandler::errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}