#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;     // RGBA color for the pixel

in vec4 projectionPosition;    // Input color from vertex shader
in vec2 TexCoords;
in vec3 toCamera;
in vec3 fromLight;
in vec4 worldPos;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform vec3 lightCol;
uniform vec3 cameraPosition;

const float waveStrength = 0.05;
uniform float moveOffset;
const float moveSpeed = 0.05;

const float shineDamper = 1;
const float reflectivity = 50;

const float near = 0.1;
const float far = 200.0;

const float fogStart = 500;
const float fogEnd = 600;

void main()
{
    // Compute device-space coordinates (could be precomputed in vertex shader)
    vec2 devicePosition = 0.5 + 0.5 * (projectionPosition.xy / projectionPosition.w);
    vec2 reflectCoords = vec2(devicePosition.x, -devicePosition.y);
    vec2 refractCoords = devicePosition;

    // Precompute depth conversion constants
    float constantFactor = 2.0 * near * far;

    // Calculate depth distances from the depth map and the current fragment
    float sceneDepth = texture(depthMap, refractCoords).r;
    float depthDistance = constantFactor / ((far + near) - (2.0 * sceneDepth - 1.0) * (far - near));

    float waterDistance = constantFactor / ((far + near) - (2.0 * gl_FragCoord.z - 1.0) * (far - near));
    float waterDepth = depthDistance - waterDistance;
    float depthFactor = min(1, waterDepth / 50.0);

    vec2 distoredTexCoords = texture(dudvMap, vec2(TexCoords.x + moveSpeed * moveOffset, TexCoords.y)).rg * 0.1;
    distoredTexCoords = TexCoords + vec2(distoredTexCoords.x, distoredTexCoords.y + moveSpeed * moveOffset);
    vec2 totalDistortion = (texture(dudvMap, distoredTexCoords).rg * 2 - 1) * waveStrength * depthFactor;

    // Adjust texture coordinates for reflection and refraction
    reflectCoords += totalDistortion;
    reflectCoords.x = clamp(reflectCoords.x, 0.0001, 0.9999);
    reflectCoords.y = clamp(reflectCoords.y, -0.9999, -0.0001);

    refractCoords += totalDistortion;
    refractCoords = clamp(refractCoords, 0.0001, 0.9999);

    // Sample reflection and refraction textures
    vec4 reflectionColor = texture(reflectionTexture, reflectCoords);
    vec4 refractionColor = texture(refractionTexture, refractCoords);

    // Fresnel effect
    float fresnel = dot(toCamera, vec3(0.0, 0.0, 1.0));

    // Compute normals from the normal map and normalize
    vec4 normalColor = texture(normalMap, distoredTexCoords);
    vec3 normal = normalize(normalColor.rgb * 2.0 - 1.0);

    // Calculate specular highlights using the reflection vector
    vec3 reflectedLight = reflect(fromLight, normal);
    float specular = max(dot(reflectedLight, toCamera), 0.0);
    specular = pow(specular, shineDamper);
    vec3 specularHighlights = lightCol * specular * reflectivity * depthFactor;

    // Blend reflection and refraction colors using the Fresnel term
    vec4 waterColor = mix(reflectionColor, refractionColor, fresnel);
    waterColor = mix(waterColor, vec4(0.0, 0.25, 0.5, 1.0), 0.10) + vec4(specularHighlights, 0.0);

    // Calculate fog factor based on distance
    float distance = length(cameraPosition - worldPos.xyz);
    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);

    // Adjust alpha based on water depth and fog
    FragColor = waterColor;
    FragColor.a = min(min(1, waterDepth / 15.0), fogFactor);
}