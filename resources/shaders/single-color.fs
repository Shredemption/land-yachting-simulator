#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 bodyColor;
uniform vec3 lightPos;
uniform vec3 lightCol;

const float ambientStrength = 0.5;

void main()
{
    vec3 ambient = ambientStrength * lightCol;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (1 - ambientStrength) * diff * lightCol;

    vec3 result = (ambient + diffuse) * bodyColor;
    FragColor = vec4(result, 1.0);
}