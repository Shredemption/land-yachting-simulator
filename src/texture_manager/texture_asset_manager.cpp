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

std::vector<std::string> TextureAssetManager::loadMaterialTexturePaths(
    const std::string &type,
    const std::string &directory)
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
            queueStandaloneTexture(texPath);

        outTexturePaths.push_back(texPath);
    }
}

void TextureAssetManager::queueStandaloneTexture(const std::string &fileName)
{
    std::string path = "resources/textures/" + fileName;

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

void TextureAssetManager::queueStandaloneImage(const std::string &fileName)
{
    std::string path = "resources/images/" + fileName;

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
    pt.repeating = false;

    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);
        textureQueue.push(std::move(pt));
    }
}

void TextureAssetManager::queueTextureToArray(
    const std::string &arrayName,
    const std::string &texturePath)
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