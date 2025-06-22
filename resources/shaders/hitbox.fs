#version 410 core

out vec4 FragColor;

uniform vec3 bodyColor;

void main()
{
    FragColor = vec4(bodyColor, 1.0);
}