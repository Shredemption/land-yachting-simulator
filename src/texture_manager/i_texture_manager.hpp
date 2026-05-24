#pragma once

#include <string>
#include <vector>

struct PendingTexture;
struct TextureArray;

class Model;

class ITextureManager
{
public:
    virtual ~ITextureManager() = default;

    virtual unsigned int createTexture2D(const PendingTexture &tex) = 0;

    virtual unsigned int createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers) = 0;

    virtual unsigned int createSkybox(const std::vector<std::string> &faces) = 0;

    virtual void uploadPending() = 0;

    virtual unsigned int getStandaloneTextureID(const std::string &texturePath) = 0;
    virtual unsigned int getStandaloneTextureUnit(const std::string &texturePath) = 0;
    virtual unsigned int getTextureArrayUnit(const std::string &arrayName) = 0;
    virtual unsigned int getTextureLayerIndex(const std::string &arrayName, const std::string &fileName) = 0;
    virtual void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers) = 0;

    virtual void clear() = 0;
};