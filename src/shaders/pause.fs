#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 texelSize;
uniform float darkenAmount;

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

    color = mix(color, vec3(0.0), darkenAmount);

    FragColor = vec4(color, 1.0);
}
