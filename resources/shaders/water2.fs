#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;     // RGBA color for the pixel

in vec4 projectionPosition;    // Input color from vertex shader
in vec3 toCamera;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;


void main()
{
    // Compute device-space coordinates (could be precomputed in vertex shader)
    vec2 devicePosition = 0.5 + 0.5 * (projectionPosition.xy / projectionPosition.w);
    vec2 reflectCoords = vec2(devicePosition.x, -devicePosition.y);
    vec2 refractCoords = devicePosition;

    // Adjust texture coordinates for reflection and refraction
    reflectCoords.x = clamp(reflectCoords.x, 0.0001, 0.9999);
    reflectCoords.y = clamp(reflectCoords.y, -0.9999, -0.0001);

    refractCoords = clamp(refractCoords, 0.0001, 0.9999);

    // Sample reflection and refraction textures
    vec4 reflectionColor = texture(reflectionTexture, reflectCoords);
    vec4 refractionColor = texture(refractionTexture, refractCoords);

    // Fresnel effect
    float fresnel = dot(toCamera, vec3(0.0, 0.0, 1.0));

    // Blend reflection and refraction colors using the Fresnel term
    vec4 waterColor = mix(reflectionColor, refractionColor, fresnel);

    // Adjust alpha based on water depth and fog
    FragColor = waterColor;
}