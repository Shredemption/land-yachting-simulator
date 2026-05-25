#pragma once

#include <mutex>
#include <unordered_map>
#include <string>

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include "i_texture_manager.hpp"

class OpenGLTextureManager : public ITextureManager
{
public:
    unsigned int createTexture2D(const PendingTexture &tex) override;
    unsigned int createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers) override;
    unsigned int uploadSkybox(const SkyboxAsset &skybox) override;

    void uploadPending() override;

    void clear() override;

    unsigned int getStandaloneTextureID(const std::string &texturePath) override;
    unsigned int getTextureArrayID(const std::string &arrayName) override;
    unsigned int getStandaloneTextureUnit(const std::string &texturePath) override;
    unsigned int getTextureArrayUnit(const std::string &arrayName) override;
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) override;
    void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers) override;

private:
    struct GPUTexture
    {
        GLuint id = 0;
        int unit = -1;
        bool ready = false;
    };

    struct GPUTextureArray
    {
        GLuint id = 0;
        int unit = -1;
        bool ready = false;
    };

    std::unordered_map<std::string, GPUTexture> gpuTextures;
    std::unordered_map<std::string, GPUTextureArray> gpuArrays;
    std::unordered_map<std::string, TextureArray *> arrayGPUMap;

    std::mutex mutex;

    std::unordered_map<std::string, GPUTexture> skyboxCache;
    int reservedBase = 5;
    int nextReservedUnit = 5;

private:
    int allocateUnit();
    GLenum formatFromChannels(int channels);
    int getSkyboxUnit();
};