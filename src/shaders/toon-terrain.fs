#version 410 core

out vec4 FragColor;

in vec4 worldPos;

const vec4 baseColor = vec4(1, 0.85, 0.51, 1.0);
const vec4 darkColor = vec4(0.81, 0.53, 0.09, 1.0);
const vec4 lightColor = vec4(0.937, 1, 1, 1);

uniform float lod;

void main()
{
    float darknessFactor = clamp(worldPos.z / 5, 0.0, 1.0);
    FragColor = mix(darkColor, baseColor, darknessFactor);
}