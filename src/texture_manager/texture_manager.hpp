#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <unordered_set>

#include "texture_manager/texture_manager_defs.h"

enum class shaderID;
enum class ModelType;
struct SkyBoxData;
class Model;

inline bool ends_with(std::string const &value, std::string const &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

namespace TextureManager
{
    // Texture Arrays
    inline std::unordered_map<std::string, TextureArray> textureArrays;
    inline std::mutex textureArrayMutex;
    inline std::unordered_map<std::string, Texture> standaloneTextureCache;
    inline std::mutex standaloneCacheMutex;

    // Pending Textures
    inline std::queue<PendingTexture> textureQueue;
    inline std::mutex textureQueueMutex;
    inline std::unordered_set<std::string> pendingTextures;
    inline std::mutex pendingTexturesMutex;
    inline std::mutex openglMutex;

    void loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName);

    unsigned int loadStandaloneTexture(const std::string &filepath);

    void queueStandaloneTexture(const std::string &fileName);
    void queueStandaloneImage(const std::string &fileName);
    void queueTextureToArray(const std::string &arrayName, const std::string &texturePath);
    void queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName);
    unsigned int loadSkyboxTexture(const SkyBoxData &skybox);

    void clearTextures();
    void uploadToGPU();
    void loadQueuedPixelData();

    std::string getTextureArrayName(ModelType modelType);
    unsigned int getStandaloneTextureID(const std::string &texturePath);
    unsigned int getTextureArrayID(const std::string &arrayName);
    unsigned int getStandaloneTextureUnit(const std::string &texturePath);
    unsigned int getTextureArrayUnit(const std::string &arrayName);
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath);
    void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers);
};