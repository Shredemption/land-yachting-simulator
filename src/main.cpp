#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}

// Keycallback to define buttom presses
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Close on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{

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
    GLFWwindow *window = glfwCreateWindow(1200, 900, "Marama", nullptr, nullptr);
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

    // Set Keycallback for window
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Get time since launch
        float time = (float)glfwGetTime();

        // Vary color based on time
        float red = (std::sin(time * 0.5f) + 1.0f) / 2.0f;
        float green = (std::sin(time * 0.3f) + 1.0f) / 2.0f;
        float blue = (std::sin(time * 0.7f) + 1.0f) / 2.0f;

        // Clear color buffer
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(red, green, blue, 1.0f);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();



    }

    //Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
