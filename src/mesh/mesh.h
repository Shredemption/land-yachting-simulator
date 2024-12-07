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

    // Mesh Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    // Mesh Renderer
    void Draw(Shader &shader);

private:
    // Rendering Data
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

#endif // MESH_H