#include "mesh/mesh.h"

#include "frame_buffer/frame_buffer.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::string shaderName)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->shader = shaderName;

    if (shaderName == "pbr")
    {
        setupPBRMesh();
    }
    else if (shaderName == "simple")
    {
        setupSimpleMesh();
    }
    else if (shaderName == "toon-water")
    {
        setupToonWaterMesh();
    }
    else if (shaderName == "water")
    {
        setupWaterMesh();
    }
    else if (shaderName == "skybox")
    {
        setupSkyBoxMesh();
    }
    else if (shaderName == "toon")
    {
        setupToonMesh();
    }
    else
    {
        setupDefaultMesh();
    }
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
    // vertex bone IDs
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, BoneIDs));
    // vertex bone weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Weights));

    glBindVertexArray(0);
}

void Mesh::setupToonMesh()
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
    // vertex bone IDs
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, BoneIDs));
    // vertex bone weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Weights));

    glBindVertexArray(0);
}

void Mesh::setupPBRMesh()
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

Mesh Mesh::genUnitPlane(glm::vec3 color, std::string shaderName)
{
    std::vector<glm::vec3> Positions = {
        {-0.5f, 0.5f, 0.0f},
        {0.5f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
    };

    std::vector<Vertex> vertices;

    std::vector<glm::vec2> TexCoords = {
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f},
    };

    for (int i = 0; i < Positions.size(); i++)
    {
        Vertex vertex;
        vertex.Position = Positions[i];
        vertex.Color = color;
        vertex.TexCoords = TexCoords[i];

        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices = {
        0, 2, 1, // First triangle
        0, 3, 2  // Second triangle
    };

    std::vector<Texture> textures;

    return Mesh(vertices, indices, textures, shaderName);
}

Mesh Mesh::genGrid(int gridSizeX, int gridSizeY, float cellSize, glm::vec3 color, std::string shaderName)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= gridSizeY; y++)
    {
        for (int x = 0; x <= gridSizeX; x++)
        {
            Vertex vertex;
            vertex.Position = glm::vec3(cellSize * (x - 0.5f * gridSizeX), cellSize * (y - 0.5f * gridSizeY), 0.0f);
            vertex.Color = color;
            vertex.TexCoords = glm::vec2((float)x / gridSizeX, (float)y / gridSizeY);

            vertices.push_back(vertex);
        }
    }

    for (int y = 0; y < gridSizeY; y++)
    {
        for (int x = 0; x < gridSizeX; x++)
        {
            int start = y * (gridSizeX + 1) + x;

            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + gridSizeX + 1);

            indices.push_back(start + 1);
            indices.push_back(start + gridSizeX + 2);
            indices.push_back(start + gridSizeX + 1);
        }
    }

    std::vector<Texture> textures;
    return Mesh(vertices, indices, textures, shaderName);
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
    // vertex colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Color));

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glBindVertexArray(0);
}

void Mesh::setupToonWaterMesh()
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

unsigned int Mesh::setupSkyBoxMesh()
{
    float skyboxVertices[] = {
        // Front face (towards -Y)
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        // Back face (towards +Y)
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        // Left face (towards -X)
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,

        // Right face (towards +X)
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // Bottom face (towards -Z)
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        // Top face (towards +Z)
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f};

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    // Bind and setup VAO/VBO
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Debugging the VAO
    GLint vaoBound;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBound);

    glBindVertexArray(0); // Unbind VAO

    return skyboxVAO;
}