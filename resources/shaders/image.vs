#version 330 core

layout(location = 0) in vec2 aPos;      // NDC quad: [-1,1] × [-1,1]
layout(location = 1) in vec2 aTexCoord; // [0,1] texture coords

uniform vec2 uPosition;   // bottom-left corner in pixels
uniform vec2 uScale;      // relative scale (1 = native size)
uniform float uRotation;  // radians, rotate about image center
uniform vec2 uImageSize;  // image dimensions in pixels
uniform vec2 uScreenSize; // screen dimensions in pixels
uniform bool uMirrored;

out vec2 TexCoord;

void main()
{
    vec2 baselineScreen = vec2(2560.0, 1440.0);
    vec2 adjustedScale = uScale * (uScreenSize / baselineScreen);

    // 1) Compute UV from NDC quad
    vec2 uv = aPos * 0.5 + 0.5;       // [0,1]

    // 2) Pixel-space offset within image
    vec2 pixelOffset = uv * uImageSize * adjustedScale;

    // 3) World pixel position before rotation
    vec2 worldPos = uPosition + pixelOffset;

    // 4) Rotate around image center
    vec2 sizeScaled = uImageSize * adjustedScale;
    vec2 center = uPosition + sizeScaled * 0.5;
    vec2 rel = worldPos - center;
    float s = sin(uRotation), c = cos(uRotation);
    mat2 R = mat2(c, -s, s, c);
    vec2 rotatedPos = center + R * rel;

    // 5) Convert to NDC: [0..screen] → [-1..1]
    vec2 ndc = (rotatedPos / uScreenSize) * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);

    float texX = uMirrored ? (1.0 - aTexCoord.x) : aTexCoord.x;
    TexCoord = vec2(texX, 1.0 - aTexCoord.y);
}
