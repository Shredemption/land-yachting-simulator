#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 Normal;
    vec2 TexCoords;
    vec3 lightDir;
    vec3 viewDir;
    vec3 halfwayDir;
}
fs_in;

uniform vec3 lightCol;
uniform float lightIntensity;

struct Material
{
    sampler2D diffuse1;
    sampler2D properties1;
};

uniform Material material;

const float ambientLight = 0.66;
const float diffuseStrength = 0.66;

float G1(float NdotX, float k)
{
    return NdotX / (max(NdotX * (1.0 - k) + k, 0.001));

}

void main()
{
    // Sample textures
    vec3 albedo = texture(material.diffuse1, fs_in.TexCoords).rgb;

    float metallic = texture(material.properties1, fs_in.TexCoords).r;
    float roughness = 1 - texture(material.properties1, fs_in.TexCoords).g;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float cosTheta = max(dot(fs_in.halfwayDir, fs_in.viewDir), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);

    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float NdotH = max(dot(fs_in.Normal, fs_in.halfwayDir), 0.0);
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    float NDF = alphaSq / (3.14159 * denom * denom);

    float k = roughness * roughness / 2.0;

    float G = G1(max(dot(fs_in.Normal, fs_in.viewDir), 0.0), k) * G1(max(dot(fs_in.Normal, fs_in.lightDir), 0.0), k);

    vec3 specular = (NDF * G * F) / (4.0 * max(dot(fs_in.Normal, fs_in.viewDir), 0.0) * max(dot(fs_in.Normal, fs_in.lightDir), 0.0) + 0.001);

    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo * lightCol * lightIntensity * max(dot(fs_in.Normal, fs_in.lightDir), 0.0);
    vec3 outputColor = albedo * ambientLight + diffuseStrength * diffuse + specular;

    FragColor = vec4(outputColor, 1.0);
}
