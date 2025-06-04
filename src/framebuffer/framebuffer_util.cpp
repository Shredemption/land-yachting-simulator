#include "framebuffer/framebuffer_util.hpp"

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include "event_handler/event_handler.hpp"
#include "render/render.hpp"

void FramebufferUtil::bindFrameBuffer(Framebuffer FBO)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO.frameBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, FBO.width, FBO.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FramebufferUtil::unbindCurrentFrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, Render::sceneFBO);
    glViewport(0, 0, EventHandler::screenWidth, EventHandler::screenHeight);
}

void FramebufferUtil::genWaterFrameBuffers()
{
    Water = true;

    glGenFramebuffers(1, &reflectionFrameBuffer);
    glGenFramebuffers(1, &refractionFrameBuffer);

    reflectionFBO = Framebuffer(EventHandler::screenWidth / 2, EventHandler::screenHeight / 2);
    bindFrameBuffer(reflectionFBO);
    reflectionFBO.createTextureAttachment();
    reflectionFBO.createDepthBufferAttachment();

    refractionFBO = Framebuffer(EventHandler::screenWidth / 2, EventHandler::screenHeight / 2);
    bindFrameBuffer(refractionFBO);
    refractionFBO.createTextureAttachment();
    refractionFBO.createDepthTextureAttachment();

    unbindCurrentFrameBuffer();
}