#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;     // RGBA color for the pixel

in vec4 projectionPosition;    // Input color from vertex shader
in vec2 TexCoords;
in vec3 toCamera;
in vec3 fromLight;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform vec3 lightCol;

const float waveStrength = 0.05;
uniform float moveOffset;
const float moveSpeed = 0.05;

const float shineDamper = 20;
const float reflectivity = 0.6;

const float near = 0.1;
const float far = 1000.0;

void main()
{
    vec2 devicePosition = 0.5 + 0.5 * (projectionPosition.xy / projectionPosition.w);
    vec2 reflectCoords = vec2(devicePosition.x, -devicePosition.y);
    vec2 refractCoords = vec2(devicePosition.x, devicePosition.y);

    float depth = texture(depthMap, refractCoords).r;
    float depthDistance = 2 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    depth = gl_FragCoord.y;
    float waterDistance = 2 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    float waterDepth = depthDistance - waterDistance;

    vec2 distoredTexCoords = texture(dudvMap, vec2(TexCoords.x + moveSpeed * moveOffset, TexCoords.y)).rg * 0.1;
    distoredTexCoords = TexCoords + vec2(distoredTexCoords.x, distoredTexCoords.y + moveSpeed * moveOffset);
    vec2 totalDistortion = (texture(dudvMap, distoredTexCoords).rg * 2 - 1) * waveStrength * min(1, waterDepth/50.0);

    reflectCoords += totalDistortion;
    reflectCoords.x = clamp(reflectCoords.x, 0.0001, 0.9999);
    reflectCoords.y = clamp(reflectCoords.y, -0.9999, -0.0001);

    refractCoords += totalDistortion;
    refractCoords = clamp(refractCoords, 0.0001, 0.9999);

    vec4 reflectionColor = texture(reflectionTexture, reflectCoords);
    vec4 refractionColor = texture(refractionTexture, refractCoords);

    float fresnel = dot(normalize(toCamera), vec3(0, 1, 0));

    vec4 normalMapColor = texture(normalMap, distoredTexCoords);
    vec3 normal = vec3(normalMapColor.x * 2 - 1, normalMapColor.z * 2 - 1, normalMapColor.y * 2 - 1);
    normal = normalize(normal);

    vec3 reflectedLight = reflect(normalize(fromLight), normal);
    float specular = max(dot(reflectedLight, normalize(toCamera)), 0.0);
    specular = pow(specular, shineDamper);
    vec3 specularHighlights = lightCol * specular * reflectivity * min(1, waterDepth/50.0); 

    FragColor = mix(reflectionColor, refractionColor, fresnel);

    FragColor = mix(FragColor, vec4(0.0, 0.25, 0.5, 1), 0.12) + vec4(specularHighlights, 0);
    FragColor.a = min(1, waterDepth/15.0);
}