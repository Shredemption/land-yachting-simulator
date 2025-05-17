#include "mesh/mesh.h"

#include "frame_buffer/frame_buffer.h"

// Constructor to store input data
template <typename VertexType>
Mesh<VertexType>::Mesh(std::vector<VertexType> &vertices, std::vector<unsigned int> &indices, std::string &shaderName)
{
    this->vertices = vertices;
    this->indices = indices;
    this->shader = shaderName;
}

template <typename VertexType>
Mesh<VertexType> Mesh<VertexType>::genUnitPlane(glm::vec3 color, std::string shaderName)
{
    std::vector<VertexType> vertices;

    std::vector<glm::vec3> Positions = {
        {-0.5f, 0.5f, 0.0f},
        {0.5f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
    };

    std::vector<glm::vec2> TexCoords = {
        {0.f, 0.f},
        {1.f, 0.f},
        {1.f, 1.f},
        {0.f, 1.f},
    };

    for (int i = 0; i < Positions.size(); i++)
    {
        VertexType vertex;
        vertex.Position = Positions[i];

        // Check vertex type and save relevant data
        if constexpr (std::is_same_v<VertexType, VertexSimple>)
        {
            vertex.Color = color;
        }
        else if constexpr (std::is_same_v<VertexType, VertexTextured>)
        {
            vertex.TexCoords = TexCoords[i];
        }

        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices = {
        0, 2, 1, // First triangle
        0, 3, 2  // Second triangle
    };

    // Return Mesh
    return Mesh(vertices, indices, shaderName);
}

template <typename VertexType>
Mesh<VertexType> Mesh<VertexType>::genGrid(int gridSizeX, int gridSizeY, float lod, glm::vec3 color, std::string shaderName)
{
    std::vector<VertexType> vertices;
    std::vector<unsigned int> indices;

    // Make hole if LOD !=0
    float holeFactor;
    if (lod == 0)
        holeFactor = 1.0f;
    else
        holeFactor = 0.25f;

    // Make vertices
    for (int y = 0; y <= gridSizeY; y++)
    {
        for (int x = 0; x <= gridSizeX; x++)
        {
            VertexType vertex;
            vertex.Position = glm::vec3(x - 0.5f * gridSizeX, y - 0.5f * gridSizeY, 0.0f);

            // Check vertex type and save relevant data
            if constexpr (std::is_same_v<VertexType, VertexSimple>)
            {
                vertex.Color = color;
            }
            else if constexpr (std::is_same_v<VertexType, VertexTextured>)
            {
                vertex.TexCoords = glm::vec2((float)x / gridSizeX, (float)y / gridSizeY);
            }

            vertices.push_back(vertex);
        }
    }

    // Make faces from vertices
    for (int y = 0; y < gridSizeY; y++)
    {
        for (int x = 0; x < gridSizeX; x++)
        {
            // If in hole, skip
            if (x > holeFactor * gridSizeX && x < gridSizeX * (1 - holeFactor) &&
                y > holeFactor * gridSizeY && y < gridSizeY * (1 - holeFactor))
            {
                continue;
            }

            int start = y * (gridSizeX + 1) + x;

            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + gridSizeX + 1);

            indices.push_back(start + 1);
            indices.push_back(start + gridSizeX + 2);
            indices.push_back(start + gridSizeX + 1);
        }
    }

    // Return Mesh
    return Mesh(vertices, indices, shaderName);
}

template <>
unsigned int Mesh<VertexSkybox>::setupSkyBoxMesh()
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

template <typename VertexType>
void Mesh<VertexType>::uploadToGPU()
{
    if constexpr (std::is_same_v<VertexType, VertexSkybox>)
    {
        setupSkyBoxMesh();
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
void Mesh<VertexType>::draw() const
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

    glEnableVertexAttribArray(1); // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSimple), (void *)offsetof(VertexSimple, Color));
}

template class Mesh<VertexAnimated>;
template class Mesh<VertexTextured>;
template class Mesh<VertexSimple>;