#pragma once

#include <array>
#include <string>
#include <vector>
#include <unordered_map>

struct Texture
{
    unsigned int index = 0;
    std::string path;
    int bindingSlot = -1;
};

struct PendingTexture
{
    std::string path;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> pixelData;
    
    bool repeating = true;
    unsigned int textureHandle = 0;
};

struct TextureArray
{
    unsigned int textureArrayHandle = 0;
    int width = 0;
    int height = 0;
    std::vector<PendingTexture> layers;
    std::unordered_map<std::string, int> textureLayerMap;

    int bindingSlot = -1;

    bool ready = false;
};

struct SkyboxCPU
{
    std::array<std::string, 6> facePaths;
};

struct SkyboxAsset
{
    std::array<PendingTexture, 6> faces;
};