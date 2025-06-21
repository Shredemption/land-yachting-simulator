#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

enum class shaderID;

template <typename VertexType>
class Mesh
{
public:
    // Local mesh data
    std::vector<VertexType> vertices;
    std::vector<unsigned int> indices;
    shaderID shader;
    unsigned int VAO, VBO, EBO;

    Mesh(std::vector<VertexType> &vertices, std::vector<unsigned int> &indices, shaderID &shader);

    void draw();
    void setupVertexAttributes();
    void uploadToGPU();
    glm::vec3 furthestInDirection(glm::vec3 direction);
};