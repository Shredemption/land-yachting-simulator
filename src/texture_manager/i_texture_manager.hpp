#pragma once

#include <string>
#include <vector>

#include "texture_manager_defs.h"

class Model;

class ITextureManager
{
public:
    virtual ~ITextureManager() = default;

    virtual void uploadPending() = 0;
    virtual unsigned int uploadSkybox(const SkyboxAsset &skybox) = 0;

    virtual void clear() = 0;

    virtual unsigned int getTextureHandle(const std::string &texturePath) = 0;
    virtual unsigned int getTextureArrayHandle(const std::string &arrayName) = 0;
    virtual int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) = 0;
    virtual void getTextureBindings(const Model &model, unsigned int &textureBinding, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices) = 0;
};