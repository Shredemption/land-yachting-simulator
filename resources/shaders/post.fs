#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform bool flipY;

void main()
{
    vec2 coords = flipY ? vec2(TexCoords.x, 1.0 - TexCoords.y) : TexCoords;
    FragColor = texture(screenTexture, coords);
}