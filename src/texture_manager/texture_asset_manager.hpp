#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>

#include "texture_manager_defs.h"

enum class shaderID;
enum class ModelType;
struct SkyBoxData;
class Model;

namespace TextureAssetManager
{
    // CPU-side storage
    inline std::unordered_map<std::string, TextureArray> textureArrays;
    inline std::mutex textureArrayMutex;

    inline std::unordered_map<std::string, Texture> standaloneTextureCache;
    inline std::mutex standaloneCacheMutex;

    // Pending CPU work
    inline std::queue<PendingTexture> textureQueue;
    inline std::mutex textureQueueMutex;

    inline std::unordered_set<std::string> pendingTextures;
    inline std::mutex pendingTexturesMutex;

    // Free unit allocator (still CPU-side bookkeeping)
    inline int nextFreeUnit = 5;
    inline std::mutex unitMutex;

    // ---- CPU-side API ----
    std::vector<std::string> loadMaterialTexturePaths(const std::string &type, const std::string &directory);
    void loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName);

    void queueStandaloneTexture(const std::string &fileName);
    void queueStandaloneImage(const std::string &fileName);

    void queueTextureToArray(const std::string &arrayName, const std::string &texturePath);
    void queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName);

    void loadQueuedPixelData();

    void clear();
}