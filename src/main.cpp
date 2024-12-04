#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "file_manager/file_manager.h"
#include "shader/shader.h"

void errorCallback(int error, const char *description)
{
    std::cerr << "GLFW Error" << error << ": " << description << std::endl;
}

// Keycallback to define buttom presses
static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Close on ESC
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void createTriangle(unsigned int &vao, unsigned int &vbo, unsigned int &ebo)
{
    // Define vertices and their colors
    // Vertex has [x, y, z] and [r, g, b]
    float triangleVertices[] = {
        0.0f, 1.0f, 0.0f,   // pos1
        1.0f, 0.0f, 0.0f,   // col1
        -1.0f, -0.5f, 0.0f, // pos2
        0.0f, 1.0f, 0.0f,   // col2
        1.0f, -0.5, 0.0f,   // pos3
        0.0f, 0.0f, 1.0f    // col3

    };

    // Define incides that represent vertex order
    unsigned int triangleIndices[] = {
        0, 1, 2};

    // Generate VertexArrayObject, VertexBufferObject, ElementBufferObject to manage and store OpenGL state
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Bind VAO to start recording OpenGL state for triangle
    glBindVertexArray(vao);

    // Bind the VBO to the GL_ARRAY_BUFFER target to store vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Upload vertex data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    // Bind the EBO to the GL_ELEMENT_ARRAY_BUFFER target to store index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // Upload index data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

    // Specify layout of vertex data:
    // Attribute 0: Position [x, y, z] - 3 floats, starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0); // Enable attr 0

    // Attribute 1: Color [r, g, b] - 3 floats, starting at offset 3 * sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // Enable attr 1

    // Unbind the VBO, EBO and VAO (optional, to prevent accidental modifications of their data)
    glBindBuffer(GL_ARRAY_BUFFER, 0); // (VBO) EBO
    glBindVertexArray(0);             // VAO
}

void createCube(unsigned int &vao, unsigned int &vbo, unsigned int &ebo)
{
    // Define vertices and their colors
    // Vertex has [x, y, z] and [r, g, b]
    float cubeVertices[] = {
        -1.f, -1.f, -1.f, // pos 1
        0.f, 0.f, 0.f,    // col 1
        -1.f, 1.f, -1.f,  // pos 2
        0.f, 1.f, 0.f,    // col 2
        1.f, 1.f, -1.f,   // pos 3
        1.f, 1.f, 0.f,    // col 3
        1.f, -1.f, -1.f,  // pos 4
        1.f, 0.f, 0.f,    // col 4
        -1.f, -1.f, 1.f,  // pos 5
        0.f, 0.f, 1.f,    // col 5
        -1.f, 1.f, 1.f,   // pos 6
        0.f, 1.f, 1.f,    // col 6
        1.f, 1.f, 1.f,    // pos 7
        1.f, 1.f, 1.f,    // col 7
        1.f, -1.f, 1.f,   // pos 8
        1.f, 0.f, 1.f,    // col 8
    };

    // Define incides that represent vertex order
    unsigned int cubeIndices[] = {
        0, 2, 1, // back
        0, 3, 2,
        0, 1, 5, // left
        0, 5, 4,
        1, 2, 6, // up
        1, 6, 5,
        2, 3, 7, // right
        2, 7, 6,
        3, 0, 4, // down
        3, 4, 7,
        4, 5, 6, // front
        4, 6, 7};

    // Generate VertexArrayObject, VertexBufferObject, ElementBufferObject to manage and store OpenGL state
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Bind VAO to start recording OpenGL state for triangle
    glBindVertexArray(vao);

    // Bind the VBO to the GL_ARRAY_BUFFER target to store vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Upload vertex data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Bind the EBO to the GL_ELEMENT_ARRAY_BUFFER target to store index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // Upload index data to GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // Specify layout of vertex data:
    // Attribute 0: Position [x, y, z] - 3 floats, starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0); // Enable attr 0

    // Attribute 1: Color [r, g, b] - 3 floats, starting at offset 3 * sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // Enable attr 1

    // Unbind the VBO, EBO and VAO (optional, to prevent accidental modifications of their data)
    glBindBuffer(GL_ARRAY_BUFFER, 0); // (VBO) EBO
    glBindVertexArray(0);             // VAO
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

    // Create triangle
    unsigned int vao, vbo, ebo;
    // createTriangle(vao, vbo, ebo);
    createCube(vao, vbo, ebo);

    // Initialize shader
    Shader simpleShader;
    simpleShader.init(
        FileManager::read("shaders/simple.vs"),
        FileManager::read("shaders/simple.fs"));

    // Get screen dimensions
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Camera Setup
    glm::vec3 cameraPosition(0.f, 0.f, 5.f);       // Camera placed at [0, 0, 2]
    glm::vec3 cameraViewDirection(0.f, 0.f, -1.f); // Camera looks in -Z axis

    // Model matrix
    glm::mat4 model(1.f); // Identity matrix, no transformation on model

    // View matrix
    glm::mat4 view = glm::lookAt(
        cameraPosition,                       // Camera Position
        cameraPosition + cameraViewDirection, // Target Position
        glm::vec3(0.f, 1.f, 0.f)              // Up vector
    );

    // Projection Matrix
    glm::mat4 projection = glm::perspective(
        (float)M_PI_2,                            // Field of view (90 deg)
        (float)screenWidth / (float)screenHeight, // Aspect Ratio (w/h)
        0.01f,                                    // Near clipping plane
        100.0f                                    // Far clipping plane
    );

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

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
        // glClearColor(red, green, blue, 1.0f);

        // Render triangle
        {
            // Activate Shader
            simpleShader.use();

            // Update model matrix for rotating on Z-axis
            model = glm::rotate(
                glm::mat4(1.f),
                // std::sin(time * 0.8f) / 2.f,
                time * 0.1f,
                glm::vec3(1.f, 0.f, 0.f));
            model = glm::rotate(
                model,
                // std::sin(time * 0.8f) / 2.f,
                time * 0.3f,
                glm::vec3(0.f, 1.f, 0.f));
            model = glm::rotate(
                model,
                // std::sin(time * 0.8f) / 2.f,
                time * 1.f,
                glm::vec3(0.f, 0.f, 1.f));

            // Set shader uniforms
            simpleShader.setMat4("u_model", model);
            simpleShader.setMat4("u_view", view);
            simpleShader.setMat4("u_projection", projection);

            // Bind the VAO that stores the triangles vertex data and settings
            glBindVertexArray(vao);

            // Issue the draw command
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // Cube
            // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0); // Triangle

            // Unbind the VAO
            glBindVertexArray(0);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup traingle
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
