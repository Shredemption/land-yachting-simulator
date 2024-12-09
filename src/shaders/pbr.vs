#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}
vs_out;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_normal;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(u_model * vec4(aPos, 1.0));

    vec3 T = normalize(mat3(u_model) * aTangent);
    vec3 N = normalize(mat3(u_normal) * aNormal);
    vec3 B = normalize(mat3(u_model) * aBitangent);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = vs_out.FragPos;
}