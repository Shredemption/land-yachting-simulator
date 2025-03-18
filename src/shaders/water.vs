#version 410 core

// Input vertex attributes
layout(location = 0) in vec3 position;  // Vertex position in local space (model space)

out vec4 projectionPosition;
out vec2 TexCoords;
out vec3 toCamera;
out vec3 fromLight;
out vec4 worldPos;

uniform vec3 lightPos;
uniform vec3 cameraPosition;

// Uniforms for transformation matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_water;

int tiling = 20;

void main() {
    // Apply the transformations to the vertex position
    worldPos = u_water * u_model * vec4(position, 1.0);
    TexCoords = worldPos.xy / tiling;
    projectionPosition = u_projection * u_view * worldPos;
    gl_Position = projectionPosition;

    toCamera = normalize(cameraPosition - worldPos.xyz);
    fromLight = normalize(worldPos.xyz - lightPos);
}