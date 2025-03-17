#version 410 core

// Input vertex attributes
layout(location = 0) in vec3 position;

out vec2 TexCoords;
out vec4 worldPos;

// Uniforms for transformation matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

const float scale = 3;

void main()
{
    // Apply the transformations to the vertex position
    worldPos = u_model * vec4(position, 1.0);
    vec3 camPos = vec3(u_view[3][0], u_view[3][1], 0);
    worldPos.xyz -= camPos;
    TexCoords = worldPos.xy / scale;
    gl_Position = u_projection * u_view * worldPos;
}