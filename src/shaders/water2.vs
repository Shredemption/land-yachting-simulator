#version 410 core

// Input vertex attributes
layout(location = 0) in vec3 position;  // Vertex position in local space (model space)

out vec4 projectionPosition;
out vec3 toCamera;

uniform vec3 cameraPosition;

// Uniforms for transformation matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_camXY;

void main() {
    // Apply the transformations to the vertex position
    vec4 worldPos = u_camXY * u_model * vec4(position, 1.0);
    projectionPosition = u_projection * u_view * worldPos;
    gl_Position = projectionPosition;

    toCamera = normalize(cameraPosition - worldPos.xyz);
}