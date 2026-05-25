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

    unsigned int getStandaloneTextureID(const std::string &texturePath) override;
    unsigned int getTextureArrayID(const std::string &arrayName) override;
    unsigned int getStandaloneTextureUnit(const std::string &texturePath) override;
    unsigned int getTextureArrayUnit(const std::string &arrayName) override;
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) override;
    void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers) override;
};