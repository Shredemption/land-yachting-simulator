#ifndef FRAMEBUFFER_UTIL_HPP
#define FRAMEBUFFER_UTIL_HPP

#include "framebuffer/framebuffer.hpp"

namespace FramebufferUtil
{
    // Global framebuffer variables
    inline bool Water = false;
    inline unsigned int reflectionFrameBuffer;
    inline unsigned int refractionFrameBuffer;
    inline Framebuffer reflectionFBO;
    inline Framebuffer refractionFBO;

    void bindFrameBuffer(Framebuffer FBO);
    void unbindCurrentFrameBuffer();
    void genWaterFrameBuffers();
};

#endif