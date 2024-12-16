#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 lightDir;
}
vs_out;

uniform vec3 lightPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normal;

uniform vec4 location_plane;

void main()
{
    vec4 worldPos = u_model * vec4(aPos, 1.0);
    gl_Position = u_projection * u_view * worldPos;

    gl_ClipDistance[0] = dot(worldPos, location_plane);

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(u_model * vec4(aPos, 1.0));
    vs_out.Normal = aNormal;
    vs_out.lightDir = normalize(lightPos - worldPos.xyz);
}