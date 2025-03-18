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
const float stepMax = 0.5;

uniform sampler2D diffuse;
uniform sampler2D shadow;

uniform float ambientLightIntensity;

void main()
{
    vec4 highlight = texture(diffuse, fs_in.TexCoords);
    vec4 shadow = texture(shadow, fs_in.TexCoords);

    float lightFactor = smoothstep(stepMin, stepMax, dot(fs_in.Normal, fs_in.lightDir));

    FragColor = mix(shadow, highlight, lightFactor);

    FragColor *= ambientLightIntensity;
}
