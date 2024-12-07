#version 410 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 ambientLightColor;
uniform vec3 diffuseLightColor;
uniform vec3 lightPos;

void main() 
{
    // Load Texture
    vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;

    // Apply ambient light
    vec3 ambient = ambientLightColor * textureColor;

    // Calculate diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos-FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseLightColor;

    vec3 result = (ambient + diffuse) * textureColor;
    FragColor = vec4(result, 1.0f);
}