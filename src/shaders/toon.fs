#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 lightDir;
}
fs_in;

const float stepMin = 0.4;
const float stepMax = 0.45;

uniform int textureLayers[4];
uniform sampler2DArray textureArray;

uniform float ambientLightIntensity;

void main()
{
    vec4 highlight = texture(textureArray, vec3(fs_in.TexCoords, textureLayers[0]));
    vec4 shadow = texture(textureArray, vec3(fs_in.TexCoords, textureLayers[1]));
    
    float lightFactor = smoothstep(stepMin, stepMax, dot(fs_in.Normal, fs_in.lightDir));

    FragColor = mix(shadow, highlight, lightFactor);

    FragColor *= ambientLightIntensity;
}
