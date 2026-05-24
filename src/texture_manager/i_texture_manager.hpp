#pragma once

#include <string>
#include <vector>

struct PendingTexture;
struct TextureArray;
struct SkyboxCPU;

class Model;

class ITextureManager
{
public:
    virtual ~ITextureManager() = default;

    virtual unsigned int createTexture2D(const PendingTexture &tex) = 0;

    virtual unsigned int createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers) = 0;

    virtual unsigned int uploadSkybox(const SkyboxCPU &skybox) = 0;

    virtual void uploadPending() = 0;

    virtual void clear() = 0;
};