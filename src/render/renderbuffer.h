#pragma once

#include <memory>

#include "mesh/meshvariant.h"
#include "render/render_defs.h"

struct RenderCommand
{
    RenderType type;

    shaderID shader;
    std::shared_ptr<std::vector<MeshVariant>> meshes;

    unsigned int textureUnit;
    unsigned int textureArrayID;
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