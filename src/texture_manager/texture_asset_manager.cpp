#include "texture_asset_manager.hpp"

#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

bool TextureAssetManager::loadTexturePixels(const std::string &path, bool forceRGBA, PendingTexture &out)
{
    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        forceRGBA ? 4 : 0);

    if (!data)
    {
        std::cerr << "Failed to load texture: " << path << "\n";
        return false;
    }

    out.path = path;
    out.width = width;
    out.height = height;
    out.channels = forceRGBA ? 4 : channels;
    out.pixelData.assign(data, data + (width * height * out.channels));

    stbi_image_free(data);
    return true;
}

void TextureAssetManager::queueStandalone(const std::string &path, bool repeating)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (standaloneAssets.find(path) != standaloneAssets.end())
        return;

    if (pendingStandalonePaths.find(path) != pendingStandalonePaths.end())
        return;

    pendingStandalonePaths.insert(path);
    standaloneQueue.emplace(path, repeating);
}

void TextureAssetManager::queueStandaloneTexture(const std::string &fileName)
{
    queueStandalone("resources/textures/" + fileName, true);
}

void TextureAssetManager::queueStandaloneImage(const std::string &fileName)
{
    queueStandalone("resources/images/" + fileName, false);
}

void TextureAssetManager::queueTextureToArray(const std::string &arrayName, const std::string &texturePath)
{
    std::lock_guard<std::mutex> lock(mutex);

    TextureArray &arr = textureArrays[arrayName];

    if (arr.textureLayerMap.find(texturePath) != arr.textureLayerMap.end())
        return;

    for (const auto &layer : arr.layers)
        if (layer.path == texturePath)
            return;

    PendingTexture tex;
    tex.path = texturePath;
    tex.pixelData.clear(); // explicit state clarity

    arr.layers.push_back(std::move(tex));
}

void TextureAssetManager::queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName)
{
    queueTextureToArray(arrayName, "resources/textures/" + fileName);
}

SkyboxAsset TextureAssetManager::loadSkybox(const SkyboxCPU &skybox)
{
    SkyboxAsset result;

    for (int i = 0; i < 6; i++)
    {
        PendingTexture tex;

        if (!loadTexturePixels(skybox.facePaths[i], true, tex))
        {
            std::cerr << "Failed skybox face: " << skybox.facePaths[i] << "\n";
            continue;
        }

        tex.repeating = false; // skyboxes should never repeat

        result.faces[i] = std::move(tex);
    }

    SceneManager::loadingProgress.first++;

    return result;
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

    for (const std::string &texPath : texturePaths)
    {
        if (std::find(outTexturePaths.begin(), outTexturePaths.end(), texPath) != outTexturePaths.end())
            continue;

        if (!textureArrayName.empty())
            queueTextureToArray(textureArrayName, texPath);
        else
            queueStandalone(texPath, true);

        outTexturePaths.push_back(texPath);
    }

    outTextureArrayName = textureArrayName;
}

void TextureAssetManager::loadQueuedPixelData()
{
    std::vector<std::pair<std::string, bool>> standaloneItems;

    {
        std::lock_guard<std::mutex> lock(mutex);

        while (!standaloneQueue.empty())
        {
            standaloneItems.push_back(standaloneQueue.front());
            standaloneQueue.pop();
        }
    }

    // -------------------------
    // Standalone textures
    // -------------------------
    for (const auto &[path, repeating] : standaloneItems)
    {
        PendingTexture tex;

        if (loadTexturePixels(path, false, tex))
        {
            tex.repeating = repeating;

            std::lock_guard<std::mutex> lock(mutex);

            standaloneAssets[path] = std::move(tex);
            pendingStandalonePaths.erase(path);

            SceneManager::loadingProgress.first++;
        }
    }

    // -------------------------
    // Texture arrays
    // -------------------------
    for (auto &[arrayName, arr] : textureArrays)
    {
        int width = 0;
        int height = 0;
        bool first = true;
        bool ok = true;

        for (auto &tex : arr.layers)
        {
            if (!tex.pixelData.empty())
                continue;

            if (!loadTexturePixels(tex.path, true, tex))
            {
                ok = false;
                continue;
            }

            if (first)
            {
                width = tex.width;
                height = tex.height;
                first = false;
            }
            else if (tex.width != width || tex.height != height)
            {
                std::cerr << "Texture array mismatch in " << arrayName << "\n";
                ok = false;
            }

            SceneManager::loadingProgress.first++;
        }

        if (ok && !arr.layers.empty())
        {
            arr.width = width;
            arr.height = height;
        }
    }
}

void TextureAssetManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex);

    while (!standaloneQueue.empty())
        standaloneQueue.pop();

    pendingStandalonePaths.clear();
    standaloneAssets.clear();

    for (auto &[name, arr] : textureArrays)
    {
        arr.layers.clear();
        arr.textureLayerMap.clear();
        arr.width = 0;
        arr.height = 0;
    }

    textureArrays.clear();
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

std::unordered_map<std::string, PendingTexture> &TextureAssetManager::getStandaloneAssets()
{
    return standaloneAssets;
}

std::unordered_map<std::string, TextureArray> &TextureAssetManager::getTextureArrays()
{
    return textureArrays;
}

SkyboxCPU TextureAssetManager::toSkyboxCPU(const SkyBoxData &data)
{
    SkyboxCPU cpu;

    cpu.facePaths = {
        data.right,
        data.left,
        data.up,
        data.down,
        data.front,
        data.back};

    return cpu;
}