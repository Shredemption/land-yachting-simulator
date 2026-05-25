#pragma once

#include <string>
#include <vector>

#include "texture_manager_defs.h"

class Model;

class ITextureManager
{
public:
    virtual ~ITextureManager() = default;

    virtual unsigned int createTexture2D(const PendingTexture &tex) = 0;
    virtual unsigned int createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers) = 0;
    virtual unsigned int uploadSkybox(const SkyboxAsset &skybox) = 0;

    virtual void uploadPending() = 0;

    virtual void clear() = 0;

    virtual unsigned int getStandaloneTextureID(const std::string &texturePath) = 0;
    virtual unsigned int getTextureArrayID(const std::string &arrayName) = 0;
    virtual unsigned int getStandaloneTextureUnit(const std::string &texturePath) = 0;
    virtual unsigned int getTextureArrayUnit(const std::string &arrayName) = 0;
    virtual int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) = 0;
    virtual void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers) = 0;
};