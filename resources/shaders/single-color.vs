#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(u_model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(u_model))) * normal;
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}