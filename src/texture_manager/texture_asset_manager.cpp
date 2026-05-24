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
        std::string fullPath = entry.path().string();

        if (filename.find(type) != std::string::npos)
        {
            for (const std::string &ext : validExtensions)
            {
                if (entry.path().extension() == ext)
                {
                    paths.push_back(fullPath);
                    break;
                }
            }
        }
    }

    return paths;
}

std::string TextureAssetManager::getTextureArrayName(ModelType modelType)
{
    switch (modelType)
    {
    case ModelType::Yacht:
        return "yachtTextureArray";
    default:
        return "";
    }
}

void TextureAssetManager::loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName)
{
    std::string textureArrayName = getTextureArrayName(modelType);
    
    std::vector<std::string> texturePaths;

    if (shader == shaderID::Default)
    {
        auto diffuseMaps = loadMaterialTexturePaths("diffuse", directory);
        texturePaths.insert(texturePaths.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto propertiesMaps = loadMaterialTexturePaths("properties", directory);
        texturePaths.insert(texturePaths.end(), propertiesMaps.begin(), propertiesMaps.end());
    }
    else if (shader == shaderID::Toon)
    {
        auto highlightMaps = loadMaterialTexturePaths("highlight", directory);
        texturePaths.insert(texturePaths.end(), highlightMaps.begin(), highlightMaps.end());

        auto shadowMaps = loadMaterialTexturePaths("shadow", directory);
        texturePaths.insert(texturePaths.end(), shadowMaps.begin(), shadowMaps.end());
    }

    for (const auto &texPath : texturePaths)
    {
        if (std::find(outTexturePaths.begin(), outTexturePaths.end(), texPath) == outTexturePaths.end())
        {
            if (!textureArrayName.empty())
                queueTextureToArray(textureArrayName, texPath);
            else
                queueStandalone(texPath, true);

            outTexturePaths.push_back(texPath);
        }
    }

    outTextureArrayName = textureArrayName;
}

void TextureAssetManager::queueStandalone(const std::string &path, bool repeating)
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

    std::cout << "[QUEUE] " << pt.path << "\n";

    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);
        textureQueue.push(std::move(pt));
    }
}

void TextureAssetManager::queueStandaloneTexture(const std::string &fileName)
{
    std::string path = "resources/textures/" + fileName;
    queueStandalone(path, true);
}

void TextureAssetManager::queueStandaloneImage(const std::string &fileName)
{
    std::string path = "resources/images/" + fileName;
    queueStandalone(path, true);
}

void TextureAssetManager::queueTextureToArray(const std::string &arrayName, const std::string &texturePath)
{
    std::lock_guard<std::mutex> arrayLock(textureArrayMutex);
    std::lock_guard<std::mutex> unitLock(unitMutex);

    TextureArray &arr = textureArrays[arrayName];

    if (arr.textureUnit == -1)
        arr.textureUnit = nextFreeUnit++;

    if (arr.textureLayerMap.find(texturePath) != arr.textureLayerMap.end())
        return;

    for (const auto &t : arr.pendingTextures)
        if (t.path == texturePath)
            return;

    PendingTexture pt;
    pt.path = texturePath;
    pt.width = 0;
    pt.height = 0;
    pt.channels = 0;
    pt.textureID = 0;
    pt.textureUnit = arr.textureUnit;

    arr.pendingTextures.push_back(std::move(pt));
}

void TextureAssetManager::queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName)
{
    std::string path = "resources/images/" + fileName;
    queueTextureToArray(arrayName, path);
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
    std::vector<std::shared_ptr<PendingTexture>> textures;

    // Standalone
    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);

        while (!textureQueue.empty())
        {
            auto pt = std::make_shared<PendingTexture>(textureQueue.front());
            textureQueue.pop();

            std::string path = pt->path;

            tasks.push_back(std::async(std::launch::async, [pt, path]()
                                       {
                int width, height, channels;
                unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

                if (!data)
                {
                    std::cerr << "Failed to load pixel data: " << path << std::endl;
                    return;
                }

                pt->width = width;
                pt->height = height;
                pt->channels = channels;
                pt->pixelData.assign(data, data + (width * height * channels));


                stbi_image_free(data);

                std::cout << "[CPU LOAD] " << pt->path << "\n";

                SceneManager::loadingProgress.first++; }));

            textures.push_back(pt);
        }
    }

    {
        std::lock_guard<std::mutex> lock(textureArrayMutex);

        for (auto &[name, arr] : textureArrays)
        {
            for (PendingTexture &ptRef : arr.pendingTextures)
            {
                PendingTexture *pt = &ptRef;

                tasks.push_back(std::async(std::launch::async, [pt]()
                                           {
                    int width, height, channels;
                    unsigned char *data = stbi_load(pt->path.c_str(), &width, &height, &channels, 4);
                   
                    if (!data)
                    {
                        std::cerr << "Failed to load pixel data: " << pt->path << std::endl;
                        return;
                    }

                    pt->width = width;
                    pt->height = height;
                    pt->channels = 4;
                    pt->pixelData.assign(data, data + (width * height * 4));

                    stbi_image_free(data);

                    SceneManager::loadingProgress.first++; }));
            }
        }
    }

    for (auto &task : tasks)
        task.get();

    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);
        for (auto &pt : textures)
            textureQueue.push(std::move(*pt));
    }
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

    if (it == textureArrays.end())
        return -1;

    auto layerIt = it->second.textureLayerMap.find(texturePath);

    if (layerIt != it->second.textureLayerMap.end())
        return layerIt->second;

    return -1;
}
void TextureAssetManager::getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers)
{
    textureLayers.clear();

    auto itArray = textureArrays.find(model.textureArrayName);

    if (itArray == textureArrays.end())
    {
        textureUnit = 0;
        textureArrayID = 0;
        return;
    }

    const TextureArray &texArray = itArray->second;

    textureUnit = texArray.textureUnit;
    textureArrayID = texArray.textureArrayID;

    for (const auto &texPath : model.texturePaths)
    {
        auto itLayer =
            texArray.textureLayerMap.find(texPath);

        if (itLayer != texArray.textureLayerMap.end())
            textureLayers.push_back(itLayer->second);
        else
            textureLayers.push_back(-1);
    }
}