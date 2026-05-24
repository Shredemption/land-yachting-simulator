#include "texture_asset_manager.hpp"

#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static bool ends_with(const std::string &value, const std::string &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> TextureAssetManager::loadMaterialTexturePaths(const std::string &type, const std::string &directory)
{
    std::vector<std::string> paths;
    const std::vector<std::string> validExtensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga"};

    if (!std::filesystem::exists(directory))
    {
        std::cerr << "Directory does not exist: " << directory << "\n";
        return paths;
    }

    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
            continue;

        std::string filename = entry.path().filename().string();

        if (filename.find(type) == std::string::npos)
            continue;

        for (const auto &ext : validExtensions)
        {
            if (entry.path().extension() == ext)
            {
                paths.push_back(entry.path().string());
                break;
            }
        }
    }

    return paths;
}

void TextureAssetManager::loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName)
{
    std::vector<std::string> texturePaths;

    if (shader == shaderID::Default)
    {
        auto d = loadMaterialTexturePaths("diffuse", directory);
        auto p = loadMaterialTexturePaths("properties", directory);
        texturePaths.insert(texturePaths.end(), d.begin(), d.end());
        texturePaths.insert(texturePaths.end(), p.begin(), p.end());
    }
    else if (shader == shaderID::Toon)
    {
        auto h = loadMaterialTexturePaths("highlight", directory);
        auto s = loadMaterialTexturePaths("shadow", directory);
        texturePaths.insert(texturePaths.end(), h.begin(), h.end());
        texturePaths.insert(texturePaths.end(), s.begin(), s.end());
    }

    for (const auto &texPath : texturePaths)
    {
        if (std::find(outTexturePaths.begin(), outTexturePaths.end(), texPath) != outTexturePaths.end())
            continue;

        if (!outTextureArrayName.empty())
            queueTextureToArray(outTextureArrayName, texPath);
        else
            queueStandalone(texPath);

        outTexturePaths.push_back(texPath);
    }
}

void TextureAssetManager::queueStandalone(const std::string &path)
{
    {
        std::lock_guard<std::mutex> lock(standaloneCacheMutex);
        if (standaloneTextureCache.find(path) != standaloneTextureCache.end())
            return;
    }

    {
        std::lock_guard<std::mutex> lock(pendingTexturesMutex);
        if (pendingTextures.find(path) != pendingTextures.end())
            return;

        pendingTextures.insert(path);
    }

    std::lock_guard<std::mutex> unitLock(unitMutex);

    PendingTexture pt;
    pt.path = path;
    pt.width = 0;
    pt.height = 0;
    pt.channels = 0;
    pt.textureID = 0;
    pt.textureUnit = nextFreeUnit++;
    pt.repeating = true;

    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);
        textureQueue.push(std::move(pt));
    }
}

void TextureAssetManager::queueStandaloneTexture(const std::string &fileName)
{
    std::string path = "resources/textures/" + fileName;
    queueStandalone(path);
}

void TextureAssetManager::queueStandaloneImage(const std::string &fileName)
{
    std::string path = "resources/images/" + fileName;
    queueStandalone(path);
}

void TextureAssetManager::queueTextureToArray(const std::string &arrayName, const std::string &texturePath)
{
    std::lock_guard<std::mutex> lock(unitMutex);

    TextureArray &arr = textureArrays[arrayName];

    if (arr.textureUnit == -1)
        arr.textureUnit = nextFreeUnit++;

    if (arr.textureLayerMap.find(texturePath) != arr.textureLayerMap.end())
        return;

    for (auto &t : arr.pendingTextures)
        if (t.path == texturePath)
            return;

    PendingTexture pt;
    pt.path = texturePath;
    pt.textureUnit = arr.textureUnit;

    arr.pendingTextures.push_back(std::move(pt));
}

void TextureAssetManager::queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName)
{
    queueTextureToArray(arrayName, "resources/textures/" + fileName);
}

SkyboxCPU TextureAssetManager::loadSkybox(const SkyBoxData &skybox)
{
    SkyboxCPU result;

    std::array<std::string, 6> paths = {
        skybox.right,
        skybox.left,
        skybox.up,
        skybox.down,
        skybox.front,
        skybox.back};

    for (size_t i = 0; i < 6; i++)
    {
        PendingTexture pt;
        pt.path = paths[i];

        int w, h, c;
        unsigned char *data = stbi_load(paths[i].c_str(), &w, &h, &c, 4);

        if (!data)
        {
            std::cerr << "Failed to load skybox face: " << paths[i] << "\n";
            continue;
        }

        pt.width = w;
        pt.height = h;
        pt.channels = 4;
        pt.pixelData.assign(data, data + (w * h * 4));

        stbi_image_free(data);

        result.faces[i] = std::move(pt);
    }

    return result;
}

void TextureAssetManager::loadQueuedPixelData()
{
    std::vector<std::future<void>> tasks;

    // Standalone
    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);

        while (!textureQueue.empty())
        {
            auto pt = std::make_shared<PendingTexture>(std::move(textureQueue.front()));
            textureQueue.pop();

            std::string path = pt->path;

            tasks.push_back(std::async(std::launch::async, [pt, path]()
                                       {
                int w, h, c;
                unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 0);

                if (!data)
                {
                    std::cerr << "Failed load: " << path << "\n";
                    return;
                }

                pt->width = w;
                pt->height = h;
                pt->channels = c;
                pt->pixelData.assign(data, data + (w * h * c));

                stbi_image_free(data); }));
        }
    }

    for (auto &t : tasks)
        t.get();
}

void TextureAssetManager::clear()
{
    std::lock_guard<std::mutex> lock1(standaloneCacheMutex);
    std::lock_guard<std::mutex> lock2(textureQueueMutex);
    std::lock_guard<std::mutex> lock3(textureArrayMutex);
    std::lock_guard<std::mutex> lock4(pendingTexturesMutex);

    standaloneTextureCache.clear();
    textureQueue = {};
    textureArrays.clear();
    pendingTextures.clear();
    nextFreeUnit = 5;
}

unsigned int TextureAssetManager::getStandaloneTextureID(const std::string &texturePath)
{
    auto it = standaloneTextureCache.find(texturePath);
    if (it != standaloneTextureCache.end())
        return it->second.index;
    return 0;
}

unsigned int TextureAssetManager::getStandaloneTextureUnit(const std::string &texturePath)
{
    auto it = standaloneTextureCache.find(texturePath);
    if (it != standaloneTextureCache.end())
        return it->second.textureUnit;
    return 0;
}

unsigned int TextureAssetManager::getTextureArrayUnit(const std::string &arrayName)
{
    auto it = textureArrays.find(arrayName);
    if (it != textureArrays.end())
        return it->second.textureUnit;
    return 0;
}

unsigned int TextureAssetManager::getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath)
{
    auto it = textureArrays.find(arrayName);
    if (it != textureArrays.end())
    {
        auto layerIt = it->second.textureLayerMap.find(texturePath);
        if (layerIt != it->second.textureLayerMap.end())
            return layerIt->second;
    }
    return -1;
}

void TextureAssetManager::getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers)
{
    textureLayers.clear();

    auto itArray = textureArrays.find(model.textureArrayName);
    if (itArray == textureArrays.end())
    {
        // Texture array not found, set default
        textureUnit = 0;
        return;
    }

    const TextureArray &texArray = itArray->second;
    textureUnit = texArray.textureUnit;
    textureArrayID = texArray.textureArrayID;

    // For each texture path in the model, find its layer index in the texture array
    for (const auto &texPath : model.texturePaths)
    {
        auto itLayer = texArray.textureLayerMap.find(texPath);
        if (itLayer != texArray.textureLayerMap.end())
        {
            textureLayers.push_back(itLayer->second);
        }
        else
        {
            textureLayers.push_back(-1);
        }
    }
}