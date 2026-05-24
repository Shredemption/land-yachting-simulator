#pragma once

#include "i_texture_manager.hpp"

class OpenGLTextureManager : public ITextureManager
{
public:
    unsigned int createTexture2D(const PendingTexture &) override;

    unsigned int createTextureArray(TextureArray &, const std::vector<PendingTexture> &) override;

    unsigned int createSkybox(const std::vector<std::string> &) override;

    void uploadPending() override;

    void clear() override;

    // optional extras (ONLY if they are in interface or temporary)
    unsigned int getStandaloneTextureID(const std::string &texturePath);
    unsigned int getStandaloneTextureUnit(const std::string &texturePath);
    unsigned int getTextureArrayUnit(const std::string &arrayName);
    unsigned int getTextureLayerIndex(const std::string &arrayName, const std::string &fileName);
    void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers);
};  