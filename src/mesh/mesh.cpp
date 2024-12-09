#include "mesh/mesh.h"
#include "frame_buffer/frame_buffer.h"

// Mesh constructor
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::string shaderName)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->shader = shaderName;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    if (shaderName == "default")
    {
        setupDefaultMesh();
    }
    else if (shaderName == "simple")
    {
        setupSimpleMesh();
    }
    else if (shaderName == "water")
    {
        setupWaterMesh();
    }
}

// Mesh Destructor
Mesh::~Mesh()
{
}

void Mesh::setupDefaultMesh()
{
    // Generate empty buffer data
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object
    glBindVertexArray(VAO);

    // Send vertices of mesh to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Send Send element indices to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

Mesh Mesh::genUnitPlane(glm::vec3 color)
{
    std::vector<glm::vec3> Positions = {
        {-0.5f, 0.0f, -0.5f},
        {0.5f, 0.0f, -0.5f},
        {0.5f, 0.0f, 0.5f},
        {-0.5f, 0.0f, 0.5f}};

    std::vector<Vertex> vertices;

    for (int i = 0; i < Positions.size(); i++)
    {
        Vertex vertex;
        vertex.Position = Positions[i];
        vertex.Color = color;

        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices = {
        0, 2, 1, // First triangle
        0, 3, 2  // Second triangle
    };

    std::vector<Texture> textures;

    return Mesh(vertices, indices, textures, "simple");
}

void Mesh::setupSimpleMesh()
{
    // Generate empty buffer data
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object
    glBindVertexArray(VAO);

    // Send vertices of mesh to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Send Send element indices to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Color));

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glBindVertexArray(0);
}

void Mesh::setupWaterMesh()
{
    // Generate empty buffer data
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind Vertex Array Object
    glBindVertexArray(VAO);

    // Send vertices of mesh to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Send Send element indices to GPU
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glBindVertexArray(0);
}
