#pragma once

#include "i_texture_manager.hpp"

class VulkanTextureManager : public ITextureManager
{
public:
    unsigned int createTexture2D(const PendingTexture &) override;
    unsigned int createTextureArray(TextureArray &, const std::vector<PendingTexture> &) override;
    unsigned int uploadSkybox(const SkyboxAsset &) override;

    void uploadPending() override;

    void clear() override;

    unsigned int getTextureHandle(const std::string &texturePath) override;
    unsigned int getTextureArrayHandle(const std::string &arrayName) override;
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) override;
    void getTextureBindings(const Model &model, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices) override;
};