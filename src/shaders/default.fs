#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 lightDir;
}
fs_in;

uniform vec3 lightCol;
uniform float lightIntensity;

struct Material
{
    sampler2D diffuse1;
};

uniform Material material;

const float ambientLight = 0.3;

void main()
{
    // Sample textures
    vec3 albedo = texture(material.diffuse1, fs_in.TexCoords).rgb;

    vec3 diffuse = dot(fs_in.Normal, fs_in.lightDir) * lightCol * lightIntensity;

    vec3 outputColor = albedo * (ambientLight + diffuse);

    FragColor = vec4(outputColor, 1.0);  // Output final color
}
