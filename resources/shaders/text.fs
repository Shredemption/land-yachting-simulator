#version 420 core
in vec2 TexCoords;  // Texture coordinates passed from vertex shader
out vec4 FragColor;  // Final color of the fragment

uniform sampler2D textTexture;  // The texture to sample
uniform vec3 textColor;  // Color of the text (to apply tint)
uniform float textAlpha;

void main()
{
    // Sample the texture at the texture coordinates
    vec4 texColor = texture(textTexture, TexCoords);
    
    // Apply the color tint to the texture (multiply the texture color by the text color)
    if (texColor.a < 0.1) {
        discard;  // Discard fully transparent fragments
    }
    
    FragColor = vec4(textColor, textAlpha) * texColor;  // Apply tint to the texture color
}