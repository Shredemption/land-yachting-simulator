#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}
fs_in;

uniform vec3 lightCol;
uniform float lightIntensity;

struct Material
{
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D ao1;
    sampler2D roughness1;
    sampler2D normal1;
};

uniform Material material;

// Helper functions for PBR terms
float distributionGGX(float NoH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    return a2 / (3.14159265359 * pow((NoH * NoH) * (a2 - 1.0) + 1.0, 2.0));
}

float geometrySmith(float NoV, float NoL, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NoL * NoV / ((NoV * (1.0 - k) + k) * (NoL * (1.0 - k) + k));
}

void main()
{
    // Sample textures
    vec3 albedo = texture(material.diffuse1, fs_in.TexCoords).rgb;
    float ao = texture(material.ao1, fs_in.TexCoords).r;
    vec3 normal = normalize(texture(material.normal1, fs_in.TexCoords).rgb * 2.0 - 1.0); // Normal map
    float roughness = texture(material.roughness1, fs_in.TexCoords).r;
    float specular = texture(material.specular1, fs_in.TexCoords).r;

    // Light and view direction
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);  // Light direction
    vec3 V = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);   // View direction
    vec3 H = normalize(L + V);  // Halfway vector

    // Dot products
    float NoL = max(dot(normal, L), 0.0);  // Light to normal
    float NoV = max(dot(normal, V), 0.0);  // View to normal
    float NoH = max(dot(normal, H), 0.0);  // Normal to halfway vector

    // Diffuse term (Lambertian reflection)
    float diff = max(NoL, 0.0);

    // Specular color (Fresnel-Schlick)
    vec3 k_s = vec3(0.04);  // Specular constant for dielectric materials (e.g., non-metal)
    vec3 specularColor = mix(k_s, albedo, specular);

    // GGX distribution and geometry terms
    float D = distributionGGX(NoH, roughness);
    float G = geometrySmith(NoV, NoL, roughness);
    vec3 F = k_s + (1.0 - k_s) * pow(1.0 - NoH, 7.0);

    // Cook-Torrance BRDF (specular reflection)
    vec3 spec = (D * G * F) * specularColor / (4.0 * NoL * NoV + 0.001);

    // Final lighting calculation
    vec3 diffuse = albedo * diff;
    vec3 ambient = albedo * 0.1 * ao;  // AO as ambient term
    vec3 finalColor = (diffuse + spec) * lightCol * lightIntensity;

    // Mix with ambient occlusion
    finalColor = mix(finalColor, ambient, 0.5);

    FragColor = vec4(finalColor, 1.0);  // Output final color
}
