#ifndef TEXTURE_MANAGER_DEFS_H
#define TEXTURE_MANAGER_DEFS_H

#include <string>
#include <vector>
#include <unordered_map>

struct Texture
{
    unsigned int index;
    std::string path;

    bool operator==(const Texture &other) const
    {
        return this->path == other.path; // Compare based on path or other identifiers
    }

    int textureUnit = -1;
};

struct PendingTexture
{
    std::string path;
    int width, height, channels;
    std::vector<unsigned char> pixelData;
    unsigned int textureID = -1;
    int textureUnit = -1;
    bool repeating;
};

struct TextureArray
{
    unsigned int textureArrayID = 0;
    int width = 0;
    int height = 0;
    std::vector<PendingTexture> pendingTextures;
    std::unordered_map<std::string, int> textureLayerMap;

    int textureUnit = -1;
};

#endif