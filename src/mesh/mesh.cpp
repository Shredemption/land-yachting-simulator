#include "mesh/mesh.hpp"

#include "pch.h"

// Constructor to store input data
template <typename VertexType>
Mesh<VertexType>::Mesh(std::vector<VertexType> &vertices, std::vector<unsigned int> &indices, shaderID &shader)
{
    this->vertices = vertices;
    this->indices = indices;
    this->shader = shader;
}

template <typename VertexType>
void Mesh<VertexType>::uploadToGPU()
{
    if constexpr (std::is_same_v<VertexType, VertexSkybox>)
    {
        MeshUtil::setupSkyBoxMesh();
    }
    else
    {
        // Generate empty buffer data
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // Bind Vertex Array Object
        glBindVertexArray(VAO);

        // Send vertices of mesh to GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType), vertices.data(), GL_STATIC_DRAW);

        // Send element indices to GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        setupVertexAttributes();

        // Unbind vertex array
        glBindVertexArray(0);
    }
}

template <typename VertexType>
void Mesh<VertexType>::draw()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

template <>
void Mesh<VertexAnimated>::setupVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAnimated), (void *)offsetof(VertexAnimated, Position));

    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAnimated), (void *)offsetof(VertexAnimated, Normal));

    glEnableVertexAttribArray(2); // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAnimated), (void *)offsetof(VertexAnimated, TexCoords));

    glEnableVertexAttribArray(3); // Bone IDs (integers!)
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexAnimated), (void *)offsetof(VertexAnimated, BoneIDs));

    glEnableVertexAttribArray(4); // Weights
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexAnimated), (void *)offsetof(VertexAnimated, Weights));
}

template <>
void Mesh<VertexTextured>::setupVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void *)offsetof(VertexTextured, Position));

    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void *)offsetof(VertexTextured, Normal));

    glEnableVertexAttribArray(2); // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void *)offsetof(VertexTextured, TexCoords));
}

template <>
void Mesh<VertexSimple>::setupVertexAttributes()
{
    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void *)offsetof(VertexSimple, Position));

    glEnableVertexAttribArray(1); // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void *)offsetof(VertexSimple, Normal));

    glEnableVertexAttribArray(2); // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void *)offsetof(VertexSimple, Color));
}

template class Mesh<VertexAnimated>;
template class Mesh<VertexTextured>;
template class Mesh<VertexSimple>;

template <typename VertexType>
glm::vec3 Mesh<VertexType>::support(glm::vec3 direction)
{
    float maxDot = -std::numeric_limits<float>::infinity();
    glm::vec3 supportPoint;

    for (const auto &vertex : vertices)
    {
        glm::vec3 pos = std::visit([](auto &&v) -> glm::vec3
                                   { return v.position; }, vertex);

        float dot = glm::dot(pos, direction)
        if (dot > maxDot)
        {
            maxDot = dot;
            supportPoint = pos;
        }
    }

    return supportPoint;
}