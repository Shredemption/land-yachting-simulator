#ifndef MESH_UTIL_HPP
#define MESH_UTIL_HPP

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>

#include "mesh/mesh.hpp"
#include "mesh/mesh_defs.h"

namespace MeshUtil
{
    // Mesh generators
    template <typename VertexType>
    Mesh<VertexType> genUnitPlane(glm::vec3 &color, shaderID &shader)
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
        return Mesh(vertices, indices, shader);
    };

    template <typename VertexType>
    Mesh<VertexType> genGrid(int gridSizeX, int gridSizeY, float lod, glm::vec3 color, shaderID &shader)
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
        return Mesh(vertices, indices, shader);
    }

    unsigned int setupSkyBoxMesh();
};

#endif