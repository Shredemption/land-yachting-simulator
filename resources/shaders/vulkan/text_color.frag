#version 450

layout(set = 0, binding = 0) uniform sampler2D fontAtlas;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 tex = texture(fontAtlas, fragUV);

    outColor = vec4(fragColor.rgb, (fragColor.a * tex.a));
}