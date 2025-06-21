#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct VertexAnimated
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    int BoneIDs[4] = {0, 0, 0, 0};
    float Weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct VertexSimple
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;
};

struct VertexTextured
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct VertexSkybox
{
    glm::vec3 Position;
};

struct VertexHitbox
{
    glm::vec3 Position;
};