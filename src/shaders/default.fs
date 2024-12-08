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

struct Material
{
    sampler2D diffuse1;
    sampler2D specular1;
    sampler2D ao1;
    sampler2D roughness1;
    sampler2D normal1;
};

uniform Material material;

// Helper functions to compute the necessary PBR terms
float distributionGGX(float NoH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float numerator = a2;
    float denominator = (NoH * NoH) * (a2 - 1.0) + 1.0;
    return numerator / (3.14159265359 * denominator * denominator);
}

float geometrySmith(float NoV, float NoL, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float ggx2 = NoL * (1.0 - k) + k;
    float ggx1 = NoV * (1.0 - k) + k;
    return NoL * NoV / (ggx1 * ggx2);
}

void main()
{
    // Textures
    vec3 albedo = texture(material.diffuse1, fs_in.TexCoords).rgb;           // Albedo
    float ao = texture(material.ao1, fs_in.TexCoords).r;                    // Ambient occlusion
    vec3 normal = texture(material.normal1, fs_in.TexCoords).rgb;             // Normal map
    float roughness = texture(material.roughness1, fs_in.TexCoords).r;      // Roughness
    float specular = texture(material.specular1, fs_in.TexCoords).r;        // Specular intensity

    // Normal calculation (from normal map)
    normal = normalize(normal * 2 - 1);
    vec3 N = normal;

    // Lighting calculations (Blinn-Phong PBR model)
    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);  // Light direction
    vec3 V = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);   // View direction
    vec3 H = normalize(L + V);               // Halfway vector

    // Lambertian Diffuse (albedo * N dot L)
    float diff = max(dot(N, L), 0.0);

    vec3 k_s = vec3(0.04);  // Specular constant for dielectric materials (e.g., metal)
    vec3 specularColor = mix(k_s, albedo, specular); // Blend between dielectric and metal

    // Specular (GGX distribution and geometry)
    float NoL = max(dot(N, L), 0.0);
    float NoV = max(dot(N, V), 0.0);
    float NoH = max(dot(N, H), 0.0);
    float D = distributionGGX(NoH, roughness);
    float G = geometrySmith(NoV, NoL, roughness);
    vec3 F = k_s + (1.0 - k_s) * pow(1.0 - NoH, 7.0);

    // Cook-Torrance BRDF for specular reflection
    vec3 spec = (D * G * F) * specularColor / (4.0 * NoL * NoV + 0.001);

    // Final color calculation
    vec3 diffuse = albedo * diff;
    vec3 ambient = albedo * 0.1 * ao; // Apply AO as a darkening factor

    vec3 finalColor = (diffuse + spec) * lightCol; // Light contribution
    finalColor = mix(finalColor, ambient, 0.5); // Combine with ambient color

    FragColor = vec4(finalColor, 1.0); // Set final fragment color
}