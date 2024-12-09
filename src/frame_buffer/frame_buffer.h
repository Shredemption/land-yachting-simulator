#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "event_handler/event_handler.h"

class FrameBuffer
{
public:
    unsigned int frameBuffer;

    FrameBuffer() : frameBuffer(0), width(0), height(0) {};
    FrameBuffer(int width, int height);

    static void bindFrameBuffer(FrameBuffer FBO);
    static void unbindCurrentFrameBuffer();

    static void WaterFrameBuffers();
    static bool Water;

    static unsigned int reflectionFrameBuffer;
    static unsigned int refractionFrameBuffer;
    static FrameBuffer reflectionFBO;
    static FrameBuffer refractionFBO;

private:
    int width;
    int height;

    int createTextureAttachment();
    int createDepthTextureAttachment();
    int createDepthBufferAttachment();
};

#endif