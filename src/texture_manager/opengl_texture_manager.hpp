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
    void uploadPending() override;
    unsigned int uploadSkybox(const SkyboxAsset &skybox) override;

    void clear() override;

    unsigned int getTextureHandle(const std::string &texturePath) override;
    unsigned int getTextureArrayHandle(const std::string &arrayName) override;
    unsigned int getStandaloneTextureUnit(const std::string &texturePath);
    unsigned int getTextureArrayUnit(const std::string &arrayName);
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) override;
    void getTextureBindings(const Model &model, unsigned int &textureBinding, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices) override;

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
    unsigned int createTexture2D(const PendingTexture &tex);
    unsigned int createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers);
};