#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <atomic>

struct Character
{
    glm::ivec2 Size;     // Size of the character
    glm::ivec2 Bearing;  // Offset from the baseline
    unsigned int Advance;      // Distance to the next character
    glm::vec4 TexCoords; // (x, y, width, height)
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