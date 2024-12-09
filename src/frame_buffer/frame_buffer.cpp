#include <glad/glad.h>
#include <iostream>

#include "frame_buffer/frame_buffer.h"

bool FrameBuffer::Water = false;
unsigned int FrameBuffer::reflectionFrameBuffer;
unsigned int FrameBuffer::refractionFrameBuffer;
FrameBuffer FrameBuffer::reflectionFBO;
FrameBuffer FrameBuffer::refractionFBO;

FrameBuffer::FrameBuffer(int w, int h)
{
    glGenFramebuffers(1, &frameBuffer);
    width = w;
    height = h;
}

void FrameBuffer::bindFrameBuffer(FrameBuffer FBO)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.frameBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, FBO.width, FBO.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::unbindCurrentFrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, EventHandler::screenWidth, EventHandler::screenHeight);
}

void FrameBuffer::WaterFrameBuffers()
{
    Water = true;

    glGenFramebuffers(1, &reflectionFrameBuffer);
    glGenFramebuffers(1, &refractionFrameBuffer);

    reflectionFBO = FrameBuffer(EventHandler::screenWidth / 8, EventHandler::screenHeight / 8);
    FrameBuffer::bindFrameBuffer(reflectionFBO);
    reflectionFBO.createTextureAttachment();
    reflectionFBO.createDepthTextureAttachment();

    refractionFBO = FrameBuffer(EventHandler::screenWidth / 4, EventHandler::screenHeight / 4);
    FrameBuffer::bindFrameBuffer(refractionFBO);
    refractionFBO.createTextureAttachment();
    refractionFBO.createDepthBufferAttachment();

    FrameBuffer::unbindCurrentFrameBuffer();
}

int FrameBuffer::createTextureAttachment()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
    
    colorTexture = texture;
    
    return texture;
};

int FrameBuffer::createDepthTextureAttachment()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

    depthTexture = texture;

    return texture;
}

int FrameBuffer::createDepthBufferAttachment()
{
    unsigned int depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, EventHandler::screenWidth, EventHandler::screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    depthRender = depthBuffer;

    return depthBuffer;
}