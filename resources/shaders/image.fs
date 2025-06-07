#version 330 core
in vec2 TexCoord;

uniform sampler2D uTexture;
uniform float uAlpha;
out vec4 FragColor;

void main()
{
    vec4 color = texture(uTexture, TexCoord);
    color.a *= uAlpha;

    FragColor = color;
}