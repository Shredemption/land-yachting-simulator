#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include <shader/shader.h>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::vec3 Color;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::string shader;
    unsigned int VAO, VBO, EBO;

    // Mesh Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::string shaderName);

    // Mesh Destructor
    ~Mesh();

    static Mesh genUnitPlane(glm::vec3 color);

private:
    // Rendering Data to GPU
    void setupPBRMesh();
    void setupDefaultMesh();
    void setupSimpleMesh();
    void setupWaterMesh();
};

#endif // MESH_H