#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 texelSize;
uniform float darkenAmount;
uniform float darkenPosition;

float map(float value, float min1, float max1, float min2, float max2)
{
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
    vec3 color = vec3(0.0);

    color += texture(screenTexture, TexCoords + texelSize * vec2(-1, -1)).rgb * 0.0625;
    color += texture(screenTexture, TexCoords + texelSize * vec2(0, -1)).rgb * 0.125;
    color += texture(screenTexture, TexCoords + texelSize * vec2(1, -1)).rgb * 0.0625;

    color += texture(screenTexture, TexCoords + texelSize * vec2(-1, 0)).rgb * 0.125;
    color += texture(screenTexture, TexCoords).rgb * 0.25;
    color += texture(screenTexture, TexCoords + texelSize * vec2(1, 0)).rgb * 0.125;

    color += texture(screenTexture, TexCoords + texelSize * vec2(-1, 1)).rgb * 0.0625;
    color += texture(screenTexture, TexCoords + texelSize * vec2(0, 1)).rgb * 0.125;
    color += texture(screenTexture, TexCoords + texelSize * vec2(1, 1)).rgb * 0.0625;

    float darken = map(smoothstep(darkenPosition - 0.1, darkenPosition + 0.1, TexCoords.x), 0, 1, darkenAmount, darkenAmount * 0.5);

    color = mix(color, vec3(0.0), darken);

    FragColor = vec4(color, 1.0);
}
