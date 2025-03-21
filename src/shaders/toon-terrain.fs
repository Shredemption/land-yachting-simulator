#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

const vec4 baseColor = vec4(1, 0.85, 0.51, 1.0);
const vec4 darkColor = vec4(0.81, 0.53, 0.09, 1.0);
const vec4 foamColor = vec4(0.937, 1, 1, 1);

uniform sampler2D heightmap;

void main()
{
    float height = texture(heightmap, TexCoord).r;
    float invDarkFactor = clamp(height * 5, 0, 1);
    float foamFactor = smoothstep(0.10, 0.11, height);

    FragColor = mix(darkColor, baseColor, invDarkFactor);
    FragColor = mix(foamColor, FragColor, foamFactor);
}