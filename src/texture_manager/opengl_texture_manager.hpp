#pragma once

#include "i_texture_manager.hpp"

class OpenGLTextureManager : public ITextureManager
{
public:
    unsigned int createTexture2D(const PendingTexture &) override;

    unsigned int createTextureArray(TextureArray &, const std::vector<PendingTexture> &) override;

    unsigned int uploadSkybox(const SkyboxCPU &) override;

    void uploadPending() override;

    void clear() override;
};