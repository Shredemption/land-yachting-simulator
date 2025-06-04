#ifndef FRAME_BUFFER_HPP
#define FRAME_BUFFER_HPP

class FrameBuffer
{
public:
    // Global framebuffer variables
    static bool Water;
    static unsigned int reflectionFrameBuffer;
    static unsigned int refractionFrameBuffer;
    static FrameBuffer reflectionFBO;
    static FrameBuffer refractionFBO;

    // Local possible framebuffer attributes
    unsigned int frameBuffer;
    unsigned int colorTexture;
    unsigned int depthTexture;
    unsigned int depthRender;

    // Default empty constructor
    FrameBuffer() : frameBuffer(0), width(0), height(0) {};

    // Actual contructor
    FrameBuffer(int width, int height);

    static void bindFrameBuffer(FrameBuffer FBO);
    static void unbindCurrentFrameBuffer();
    static void WaterFrameBuffers();

private:
    // Size variables
    int width;
    int height;

    // Create attachments
    int createTextureAttachment();
    int createDepthTextureAttachment();
    int createDepthBufferAttachment();
};

#endif