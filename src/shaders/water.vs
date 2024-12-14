#version 410 core

// Input vertex attributes
layout(location = 0) in vec3 position;  // Vertex position in local space (model space)
layout(location = 1) in vec2 texCoords;

out vec4 projectionPosition;
out vec2 TexCoords;
out vec3 toCamera;
out vec3 fromLight;
out vec4 worldPos;

uniform vec3 lightPos;
uniform vec3 cameraPosition;

// Uniforms for transformation matrices
uniform mat4 u_model;           // Model Matrix: transforms from local to world space
uniform mat4 u_view;            // View Matrix: transforms from world space to camera/view space
uniform mat4 u_projection;      // Projection matrix: transforms from camera space to clip space

int tiling = 8;

void main()
{
    // Apply the transformations to the vertex position
    worldPos = u_model * vec4(position, 1.0);
    projectionPosition = u_projection * u_view * worldPos;
    gl_Position = projectionPosition;
    TexCoords = texCoords * tiling;

    toCamera = cameraPosition - worldPos.xyz;
    fromLight = worldPos.xyz - lightPos;
}