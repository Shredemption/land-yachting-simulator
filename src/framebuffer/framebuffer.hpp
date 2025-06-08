#pragma once

class Framebuffer
{
public:
    // Local possible framebuffer attributes
    unsigned int frameBuffer;
    unsigned int colorTexture;
    unsigned int depthTexture;
    unsigned int depthRender;

    // Default empty constructor
    Framebuffer() : frameBuffer(0), width(0), height(0) {};

    // Actual contructor
    Framebuffer(int width, int height);

    // Size variables
    int width;
    int height;

    // Create attachments
    int createTextureAttachment();
    int createDepthTextureAttachment();
    int createDepthBufferAttachment();
};