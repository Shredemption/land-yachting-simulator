#version 410 core

out vec4 FragColor;     // RGBA color for the pixel

in vec4 worldPos;
in vec2 waterTexCoords;
in vec2 heightTexCoords;

uniform vec3 cameraPosition;

uniform sampler2D toonWater;
uniform sampler2D normalMap;
uniform sampler2D heightmap;

const vec4 baseColor = vec4(0.078, 0.416, 1, 1.0);
const vec4 darkColor = vec4(0.078, 0.38, 0.89, 1.0);
const vec4 lightColor = vec4(0.937, 1, 1, 1);

const float moveSpeed = 5;
const float moveScale = 1.0 / 400.0;
uniform float moveOffset;

const float fogStart = 450;
const float fogEnd = 600;

void main()
{
    vec3 normal = texture(normalMap, moveScale * vec2(waterTexCoords.x + moveSpeed * moveOffset, waterTexCoords.y)).rgb;
    float height = smoothstep(0.998, 0.999, 1-texture(heightmap, heightTexCoords).r);

    float darks = texture(toonWater, waterTexCoords + normal.xy).r;
    float lights = texture(toonWater, waterTexCoords + normal.xz).r;

    FragColor = mix(baseColor, darkColor, darks);
    FragColor = mix(FragColor, lightColor, lights);
    FragColor = mix(lightColor, FragColor, height);

    float distance = length(cameraPosition - worldPos.xyz);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);

    FragColor.a = fogFactor;
}