#version 410 core

// Input vertex attributes
layout(location = 0) in vec3 position;

out vec4 worldPos;
out vec2 waterTexCoords;
out vec2 heightTexCoords;

// Uniforms for transformation matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_camXY;

const float waterScale = 5;
const float heightScale = 1024;

void main()
{
    // Apply the transformations to the vertex position
    worldPos = u_camXY * u_model * vec4(position, 1.0);

    waterTexCoords = worldPos.xy / waterScale;

    heightTexCoords = worldPos.xy / (heightScale) + vec2(0.5);

    gl_Position = u_projection * u_view * worldPos;
}