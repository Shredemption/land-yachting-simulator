#version 410 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray sandTextureArray;

uniform vec3 lightPos;
uniform vec3 lightCol;

const float texScale = 100;

void main()
{
    vec4 color = texture(sandTextureArray, vec3(TexCoord * texScale, 0));
    float roughness = texture(sandTextureArray, vec3(TexCoord * texScale, 1)).r;
    vec3 normal = texture(sandTextureArray, vec3(TexCoord * texScale, 2)).rgb * 2.0 - 1.0;

    normal = normalize(normal);

    float intensity = max(dot(normal, normalize(lightPos)), 0.0) * 0.1 + 0.9;

    vec3 diffuse = color.rgb * lightCol * intensity;

    FragColor = vec4(diffuse, 1.0);
}