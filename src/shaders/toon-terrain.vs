#version 410 core

layout(location = 0) in vec3 aPosition;

out vec2 TexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_camXY;

uniform sampler2D heightmap;

uniform float lod;

const float scale = 1024;

void main()
{
    vec4 worldPos = u_camXY * u_model * vec4(aPosition, 1.0);
    TexCoord = worldPos.xy / (scale) + vec2(0.5);
    float height = texture(heightmap, TexCoord).r;
    worldPos.z = 3 * height - lod / 24 - 1;
    gl_Position = u_projection * u_view * worldPos;
}