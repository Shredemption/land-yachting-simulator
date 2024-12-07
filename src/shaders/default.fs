#version 410 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

uniform float specularStrength;

void main()
{
    // Load Texture
    vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;

    // Apply ambient light
    vec3 ambient = lightColor * textureColor;

    // Calculate diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(FragPos - lightPos);

    float diff = max(dot(-norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Calculate Specular lighting
    vec3 viewDir = normalize(FragPos - camPos);
    vec3 reflectDir = reflect(lightDir, norm);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * textureColor;
    FragColor = vec4(result, 1.0);
}