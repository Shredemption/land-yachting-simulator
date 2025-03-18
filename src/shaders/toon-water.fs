#version 410 core

out vec4 FragColor;     // RGBA color for the pixel

in vec2 TexCoords;
in vec4 worldPos;

uniform vec3 cameraPosition;

uniform sampler2D toonWater;
uniform sampler2D normalMap;

const vec4 baseColor = vec4(0.078, 0.416, 1, 1.0);
const vec4 darkColor = vec4(0.078, 0.38, 0.89, 1.0);
const vec4 lightColor = vec4(0.937, 1, 1, 1);

const float moveSpeed = 5;
const float moveScale = 1.0f/400.0f;
uniform float moveOffset;

const float fogStart = 450;
const float fogEnd = 600;

void main()
{
    vec3 normal = texture(normalMap, moveScale * vec2(TexCoords.x + moveSpeed * moveOffset, TexCoords.y)).rgb;

    float darks = texture(toonWater, TexCoords + normal.xy).r;
    float lights = texture(toonWater, TexCoords + normal.xz).r;

    FragColor = mix(baseColor, darkColor, darks);
    FragColor = mix(FragColor, lightColor, lights);

    float distance = length(cameraPosition - worldPos.xyz);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);

    FragColor.a = fogFactor;
}