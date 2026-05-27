#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <atomic>
#include <memory>

#include "mesh/meshvariant.h"

struct Character
{
    glm::ivec2 Size;      // Size of the character
    glm::ivec2 Bearing;   // Offset from the baseline
    unsigned int Advance; // Distance to the next character
    glm::vec4 TexCoords;  // (x, y, width, height)
};

enum class BufferState
{
    Free,
    Prepping,
    Ready,
    Rendering
};

enum class RenderType
{
    Model,
    Hitbox,
    OpaquePlane,
    TransparentPlane,
    Grid
};

enum class TextAlign
{
    Left,
    Right,
    Center
};

struct RenderCommand
{
    RenderType type;

    shaderID shader;
    glm::vec3 color;
    std::shared_ptr<std::vector<MeshVariant>> meshes;

    unsigned int textureBinding;
    unsigned int textureArrayHandle;
    std::vector<int> textureLayers;

    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;

    int lod;

    bool animated = false;
    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> boneInverseOffsets;
};

struct RenderBuffer
{
    std::vector<RenderCommand> commandBuffer;
    std::atomic<BufferState> state = BufferState::Free;
    float camYaw;
    glm::vec3 camPos;
};