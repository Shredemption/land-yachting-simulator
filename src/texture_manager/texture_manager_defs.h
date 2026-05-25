#pragma once

#include <array>
#include <string>
#include <vector>
#include <unordered_map>

struct Texture
{
    unsigned int index = 0;
    std::string path;
    int textureUnit = -1;
};

struct PendingTexture
{
    std::string path;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> pixelData;
    
    bool repeating = true;
    unsigned int textureID = 0;
    int textureUnit = -1;
};

struct TextureArray
{
    unsigned int textureArrayID = 0;
    int width = 0;
    int height = 0;
    std::vector<PendingTexture> layers;
    std::unordered_map<std::string, int> textureLayerMap;

    int textureUnit = -1;

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