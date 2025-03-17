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
    // Initialize the final position of the vertex
    vec4 finalPosition = vec4(aPos, 1);
    vec3 finalNormal = vec3(aNormal);

    vec4 worldPosition = u_model * finalPosition;

    gl_ClipDistance[0] = dot(worldPosition, location_plane);

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = worldPosition.xyz;
    vs_out.Normal = normalize(mat3(u_normal) * finalNormal);
    vs_out.lightDir = normalize(lightPos - worldPosition.xyz);

    gl_Position = u_projection * u_view * worldPosition;
}