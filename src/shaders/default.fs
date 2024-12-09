#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
}
fs_in;

uniform vec3 lightCol;
uniform float lightIntensity;

struct Material
{
    sampler2D diffuse1;
};

uniform Material material;

void main()
{
    // Sample textures
    vec3 albedo = texture(material.diffuse1, fs_in.TexCoords).rgb;

    FragColor = vec4(albedo, 1.0);  // Output final color
}
