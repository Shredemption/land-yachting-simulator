#include <iostream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "file_manager/file_manager.h"
#include "shader/shader.h"
#include "model.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

void errorCallback(int error, const char *description);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void processInput(GLFWwindow *window);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);

// Global Camera Variables
glm::vec3 worldUp(0.f, 1.f, 0.f);        // World up direction [y]
glm::vec3 cameraPosition(0.f, 0.f, 5.f); // Camera placed at [0, 0, 2]
float yaw = 0, pitch = 0, roll = 0;
float xLast, yLast;
glm::vec3 cameraViewDirection(0.0f, 0.0f, -1.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, -cameraViewDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(-cameraViewDirection, cameraRight));

// Global Time
float deltaTime;
float lastTime;

// Global screen variables
int xPos, yPos, screenWidth, screenHeight;
bool fullscreen = false;
GLFWmonitor *monitor;
int windowXpos, windowYpos, windowWidth, windowHeight;

int main()
{

    // Flip textures vertically on load
    stbi_set_flip_vertically_on_load(true);

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create Window
    GLFWwindow *window = glfwCreateWindow(800, 600, "Marama", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(window);

    // Set swap interval
    glfwSwapInterval(1);

    // GLAD loads all OpenGL pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Open Model
    Model objModel(FileManager::getPath("resources/objects/backpack/backpack.obj"));

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Initialize shader
    Shader modelShader;
    modelShader.init(FileManager::read("shaders/model.vs"), FileManager::read("shaders/model.fs"));
    // Shader modelShader("shaders/model.vs", "shaders/model.fs");

    // Get screen dimensions
    glfwGetWindowPos(window, &windowXpos, &windowYpos);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Set initial mouse position
    double initialMouseX, initialMouseY;
    glfwGetCursorPos(window, &initialMouseX, &initialMouseY);
    xLast = static_cast<float>(initialMouseX);
    yLast = static_cast<float>(initialMouseY);

    // Set Keycallback for window
    glfwSetKeyCallback(window, keyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enalbe Depth buffer (Z-buffer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Set window to fullscreen by default
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    fullscreen = true;

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            std::cout << "Window minimized, pauzing..." << std::endl;
            glfwWaitEvents();
            continue;
        }
        else
        {
            // Get time since launch
            float time = (float)glfwGetTime();
            deltaTime = time - lastTime;
            lastTime = time;

            // Clear color buffer
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Process Inputs
            processInput(window);

            // Projection Matrix
            glm::mat4 projection = glm::perspective((float)M_PI_2,                            // Field of view (90 deg)
                                                    (float)screenWidth / (float)screenHeight, // Aspect Ratio (w/h)
                                                    0.01f,                                    // Near clipping plane
                                                    100.0f                                    // Far clipping plane
            );

            // View matrix
            glm::mat4 view = glm::lookAt(cameraPosition,                       // Camera Position
                                         cameraPosition + cameraViewDirection, // Target Position
                                         cameraUp                              // Up vector
            );

            // Model Matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f));

            modelShader.setMat4("u_projection", projection);
            modelShader.setMat4("u_view", view);
            modelShader.setMat4("u_model", model);
            modelShader.use();

            objModel.Draw(modelShader);

            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}

// Keycallback to define buttom presses
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    // Find how much mouse moved
    float xOffset = xPos - xLast;
    float yOffset = yPos - yLast;
    xLast = xPos;
    yLast = yPos;

    // Apply sensitivity
    float sensitvity = 0.1f;
    xOffset *= sensitvity;
    yOffset *= sensitvity;

    // Update yaw and pitch
    yaw += glm::radians(xOffset); // Convert to radians
    pitch += glm::radians(yOffset);
    // roll += 0;

    // Clamp the pitch to prevent flipping
    if (pitch > glm::radians(85.0f)) // Maximum upward angle
        pitch = glm::radians(85.0f);
    if (pitch < glm::radians(-85.0f)) // Maximum downward angle
        pitch = glm::radians(-85.0f);

    // Generate new direction vector(s)
    cameraViewDirection = glm::normalize(glm::vec3(cos(-pitch) * sin(-yaw + glm::radians(180.f)), sin(-pitch),
                                                   cos(-pitch) * cos(-yaw + glm::radians(180.f))));
}

void processInput(GLFWwindow *window)
{
    // Move cam with WASD, space, shift
    float cameraSpeed = 5.f * deltaTime;

    // Find XZ plane view direction
    glm::vec3 forwardXZ = cameraViewDirection;
    forwardXZ.y = 0.f;
    forwardXZ = glm::normalize(forwardXZ);

    // Apply correct movement per button pressed
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * forwardXZ;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * forwardXZ;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(forwardXZ, worldUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(forwardXZ, worldUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPosition += cameraSpeed * worldUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * worldUp;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}