#version 410 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 camPos;

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shinyness;
};

uniform Light light;
uniform Material material;

void main()
{
    // Load Texture
    vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;

    // Apply ambient light
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    // Calculate diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(FragPos - light.position);

    float diff = max(dot(-norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)); 

    // Calculate Specular lighting
    vec3 viewDir = normalize(FragPos - camPos);
    vec3 reflectDir = reflect(lightDir, norm);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

    vec3 result = (ambient + diffuse + specular) * textureColor;
    FragColor = vec4(result, 1.0);
}