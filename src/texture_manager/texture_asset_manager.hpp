#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>
#include <filesystem>
#include <iostream>
#include <algorithm>

#include "texture_manager_defs.h"

enum class shaderID;
enum class ModelType;
struct SkyBoxData;
class Model;

namespace TextureAssetManager
{
    void loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName);

    void queueTexture(const std::string &fileName);
    void queueImage(const std::string &fileName);
    void queueTextureInternal(const std::string &path, bool repeating);

    void queueTextureToArray(const std::string &arrayName, const std::string &texturePath);
    void queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName);

    SkyboxAsset loadSkybox(const SkyboxCPU &skybox);
    SkyboxCPU toSkyboxCPU(const SkyBoxData &data);

    void resolveQueuedTextures();

    void clear();

    std::string getTextureArrayName(ModelType modelType);

    std::unordered_map<std::string, PendingTexture> &getStandaloneTextures();
    std::unordered_map<std::string, TextureArray> &getTextureArrays();

    std::vector<std::string> loadMaterialTexturePaths(const std::string &type, const std::string &directory);
    bool loadTexturePixels(const std::string &path, bool forceRGBA, PendingTexture &out);

    inline std::mutex mutex;

    inline std::queue<std::pair<std::string, bool>> textureQueue;
    inline std::unordered_set<std::string> pendingTexturePaths;

    inline std::unordered_map<std::string, PendingTexture> standaloneTextures;
    inline std::unordered_map<std::string, TextureArray> textureArrays;
};