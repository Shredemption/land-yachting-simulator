#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;     // RGBA color for the pixel

in vec3 vertexColor;    // Input color from vertex shader

void main(){
    // Set output fragment color
    // Convert the vec3 color to vec4, with alpha = 1
    FragColor = vec4(vertexColor, 0.5);
}