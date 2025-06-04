#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>

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

    // Mesh generators
    static Mesh<VertexType> genUnitPlane(glm::vec3 &color, shaderID &shader);
    static Mesh<VertexType> genGrid(int gridSizeX, int gridSizeY, float lod, glm::vec3 color, shaderID &shader);
    static unsigned int setupSkyBoxMesh();

    void draw() const;

    // Send mesh data to gpu
    void setupVertexAttributes();
    void uploadToGPU();
};
    
#endif