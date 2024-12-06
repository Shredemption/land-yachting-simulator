#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 ambientLightColor;

void main() 
{
    // Load Texture
    vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;

    // Apply ambient light
    vec3 ambient = ambientLightColor * textureColor;

    vec3 result = ambient * textureColor;
    FragColor = vec4(result, 1.0f);
}