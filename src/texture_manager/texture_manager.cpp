#include "texture_manager/texture_manager.hpp"

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <future>

#include "model/model.hpp"
#include "scene/scene_defs.h"
#include "scene_manager/scene_manager.hpp"

int nextFreeUnit = 5;
std::mutex unitMutex;

std::vector<std::string> loadMaterialTexturePaths(const std::string &type, const std::string &directory)
{
    std::vector<std::string> paths;
    const std::vector<std::string> validExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga"};

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

        // Check if filename contains the typeName and has a valid extension
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

void TextureManager::loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName)
{
    std::string textureArrayName = getTextureArrayName(modelType);

    std::vector<std::string> texturePaths;

    if (shader == shaderID::shDefault)
    {
        auto diffuseMaps = loadMaterialTexturePaths("diffuse", directory);
        texturePaths.insert(texturePaths.end(), diffuseMaps.begin(), diffuseMaps.end());

        auto propertiesMaps = loadMaterialTexturePaths("properties", directory);
        texturePaths.insert(texturePaths.end(), propertiesMaps.begin(), propertiesMaps.end());
    }
    else if (shader == shaderID::shToon)
    {
        auto highlightMaps = loadMaterialTexturePaths("highlight", directory);
        texturePaths.insert(texturePaths.end(), highlightMaps.begin(), highlightMaps.end());

        auto shadowMaps = loadMaterialTexturePaths("shadow", directory);
        texturePaths.insert(texturePaths.end(), shadowMaps.begin(), shadowMaps.end());
    }

    for (const std::string &texPath : texturePaths)
    {
        if (std::find(outTexturePaths.begin(), outTexturePaths.end(), texPath) == outTexturePaths.end())
        {
            if (!textureArrayName.empty())
            {
                TextureManager::queueTextureToArray(textureArrayName, texPath);
            }
            else
            {
                TextureManager::queueStandaloneTexture(texPath);
            }

            outTexturePaths.push_back(texPath);
        }
    }

    outTextureArrayName = textureArrayName;
}

unsigned int TextureManager::loadStandaloneTexture(const std::string &filepath)
{
    {
        // Check cache first
        std::lock_guard<std::mutex> lock(standaloneCacheMutex);
        auto it = standaloneTextureCache.find(filepath);
        if (it != standaloneTextureCache.end())
        {
            // Texture already loaded, return cached texture ID
            return it->second.index;
        }
    }

    // Load texture as usual if not cached
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return 0;
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else
    {
        std::cerr << "Unknown number of channels (" << nrChannels << ") in texture: " << filepath << std::endl;
        stbi_image_free(data);
        return 0;
    }

    unsigned int textureID;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    {
        std::lock_guard<std::mutex> lock(standaloneCacheMutex);
        Texture texture;
        texture.path = filepath;
        texture.index = textureID;
        texture.textureUnit = 0;
        standaloneTextureCache[filepath] = texture; // cache it
    }

    return textureID;
}

void queueStandalone(const std::string &path, bool repeating)
{
    {
        std::lock_guard<std::mutex> lock(TextureManager::standaloneCacheMutex);
        if (TextureManager::standaloneTextureCache.find(path) != TextureManager::standaloneTextureCache.end())
        {
            // Already loaded
            return;
        }
    }

    {
        std::lock_guard<std::mutex> lock(TextureManager::pendingTexturesMutex);
        if (TextureManager::pendingTextures.find(path) != TextureManager::pendingTextures.end())
        {
            // Already queued for loading
            return;
        }

        // Mark as pending
        TextureManager::pendingTextures.insert(path);
    }

    std::lock_guard<std::mutex> unitLock(unitMutex);
    PendingTexture pt;
    pt.path = path;
    pt.width = 0;
    pt.height = 0;
    pt.channels = 0;
    pt.textureID = 0;
    pt.textureUnit = nextFreeUnit++;
    pt.repeating = repeating;

    {
        std::lock_guard<std::mutex> lock(TextureManager::textureQueueMutex);
        TextureManager::textureQueue.push(std::move(pt));
    }
}

void TextureManager::queueStandaloneTexture(const std::string &fileName)
{
    std::string path = "resources/textures/" + fileName;
    queueStandalone(path, true);
}

void TextureManager::queueStandaloneImage(const std::string &fileName)
{
    std::string path = "resources/images/" + fileName;
    queueStandalone(path, false);
}

void TextureManager::queueTextureToArray(const std::string &arrayName, const std::string &texturePath)
{
    std::lock_guard<std::mutex> unitLock(unitMutex);
    TextureArray &arr = textureArrays[arrayName]; // creates if doesn't exist

    if (arr.textureUnit == -1)
    {
        arr.textureUnit = nextFreeUnit++;
    }

    if (arr.textureLayerMap.count(texturePath) > 0)
        return; // Already in map

    for (const auto &tex : arr.pendingTextures)
        if (tex.path == texturePath)
            return; // Already queued

    PendingTexture pt;
    pt.path = texturePath;
    pt.width = 0;
    pt.height = 0;
    pt.channels = 0;
    pt.textureID = 0;
    pt.textureUnit = arr.textureUnit;

    arr.pendingTextures.push_back(std::move(pt));
}

void TextureManager::queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName)
{
    static const std::string baseTextureDir = "resources/textures/";
    std::string fullPath = baseTextureDir + fileName;
    queueTextureToArray(arrayName, fullPath);
}

unsigned int TextureManager::loadSkyboxTexture(const SkyBoxData &skybox)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    std::vector<std::string> faces = {
        skybox.right,
        skybox.left,
        skybox.up,
        skybox.down,
        skybox.front,
        skybox.back};

    int width, height, channels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &channels, 4);
        if (data)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load skybox texture: " << faces[i] << std::endl;
            stbi_image_free(data);
            // Optional: could delete textureID here and return 0
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

void TextureManager::loadQueuedPixelData()
{
    std::vector<std::future<void>> tasks;
    std::vector<std::shared_ptr<PendingTexture>> textures;

    // Standalones
    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);

        while (!textureQueue.empty())
        {
            auto pt = std::make_shared<PendingTexture>(std::move(textureQueue.front()));
            textureQueue.pop();

            std::string path = pt->path;

            tasks.push_back(std::async(std::launch::async, [pt, path]()
                                       {
                int width, height, channels;
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

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
                
                SceneManager::loadingProgress.first++; }));

            textures.push_back(pt);
        }
    }

    // Texture Arrays
    {
        std::lock_guard<std::mutex> lock(textureArrayMutex);

        for (auto &[arrayName, array] : textureArrays)
        {
            for (PendingTexture &pt : array.pendingTextures)
            {
                tasks.push_back(std::async(std::launch::async, [&pt]()
                                           {
                    int width, height, channels;
                    unsigned char* data = stbi_load(pt.path.c_str(), &width, &height, &channels, 4);
                    if (!data)
                    {
                        std::cerr << "Failed to load pixel data: " << pt.path << std::endl;
                        return;
                    }

                    pt.width = width;
                    pt.height = height;
                    pt.channels = 4;
                    pt.pixelData.assign(data, data + (width * height * 4));
                    stbi_image_free(data); 
                
                    SceneManager::loadingProgress.first++; }));
            }
        }
    }

    // Wait for all loading to finish
    for (auto &task : tasks)
    {
        task.get();
    }

    // Now all textures are loaded, move them back to the queue safely
    {
        std::lock_guard<std::mutex> lock(textureQueueMutex);

        for (auto &pt : textures)
        {
            textureQueue.push(std::move(*pt));
        }
    }
}

void uploadStandalones()
{
    std::lock_guard<std::mutex> lock(TextureManager::textureQueueMutex);

    while (!TextureManager::textureQueue.empty())
    {
        PendingTexture &pending = TextureManager::textureQueue.front();

        unsigned int texID;
        glActiveTexture(GL_TEXTURE0 + pending.textureUnit);
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        GLenum format = GL_RGBA; // or deduce from channels if stored

        // Choose format based on pending.channels if you store it in PendingTexture
        switch (pending.channels)
        {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            format = GL_RGBA;
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, pending.width, pending.height, 0, format, GL_UNSIGNED_BYTE, pending.pixelData.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pending.repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pending.repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        {
            Texture tex;
            tex.path = pending.path;
            tex.index = texID;
            tex.textureUnit = pending.textureUnit;

            std::lock_guard<std::mutex> cacheLock(TextureManager::standaloneCacheMutex);
            TextureManager::standaloneTextureCache[pending.path] = tex;
        }

        // Optionally clear pixel data to free memory
        pending.pixelData.clear();
        pending.pixelData.shrink_to_fit();

        // Remove from pendingTextures set so it can be loaded again if needed
        {
            std::lock_guard<std::mutex> pendingLock(TextureManager::pendingTexturesMutex);
            TextureManager::pendingTextures.erase(pending.path);
        }

        TextureManager::textureQueue.pop();
    }
}

void uploadTextureArrays()
{
    std::lock_guard<std::mutex> lock(TextureManager::openglMutex);

    for (auto &[arrayName, arr] : TextureManager::textureArrays)
    {
        if (arr.textureArrayID != 0)
            continue; // Already uploaded

        int layerCount = (int)arr.pendingTextures.size();
        if (layerCount == 0)
            continue;

        arr.width = arr.pendingTextures[0].width;
        arr.height = arr.pendingTextures[0].height;

        glActiveTexture(GL_TEXTURE0 + arr.textureUnit);
        glGenTextures(1, &arr.textureArrayID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, arr.textureArrayID);

        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, arr.width, arr.height, layerCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        for (int layer = 0; layer < layerCount; ++layer)
        {
            const PendingTexture &tex = arr.pendingTextures[layer];
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, layer,
                            tex.width, tex.height, 1,
                            GL_RGBA, GL_UNSIGNED_BYTE, tex.pixelData.data());
            arr.textureLayerMap[tex.path] = layer;
        }

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Clear pending textures since now uploaded
        arr.pendingTextures.clear();
    }
}

void TextureManager::uploadToGPU()
{
    uploadStandalones();
    uploadTextureArrays();
}

void clearStandaloneCache()
{
    std::lock_guard<std::mutex> lock(TextureManager::standaloneCacheMutex);
    for (auto &pair : TextureManager::standaloneTextureCache)
    {
        glDeleteTextures(1, &pair.second.index);
    }
    TextureManager::standaloneTextureCache.clear();
}

void clearTextureArrays()
{
    for (auto &[name, texArray] : TextureManager::textureArrays)
    {
        if (texArray.textureArrayID != 0)
        {
            glDeleteTextures(1, &texArray.textureArrayID);
            texArray.textureArrayID = 0;
        }
        texArray.textureLayerMap.clear();
        texArray.pendingTextures.clear();
    }
    TextureManager::textureArrays.clear();
}

void TextureManager::clearTextures()
{
    clearStandaloneCache();
    clearTextureArrays();
    nextFreeUnit = 5;
}

std::string TextureManager::getTextureArrayName(ModelType modelType)
{
    switch (modelType)
    {
    case ModelType::mtYacht:
        return "yachtTextureArray";
    default:
        return ""; // Empty goes to standalone
    }
}

unsigned int TextureManager::getStandaloneTextureID(const std::string &texturePath)
{
    auto it = standaloneTextureCache.find(texturePath);
    if (it != standaloneTextureCache.end())
        return it->second.index;
    return 0;
}

unsigned int TextureManager::getTextureArrayID(const std::string &arrayName)
{
    auto it = textureArrays.find(arrayName);
    if (it != textureArrays.end())
        return it->second.textureArrayID;
    return 0;
}

unsigned int TextureManager::getStandaloneTextureUnit(const std::string &texturePath)
{
    auto it = standaloneTextureCache.find(texturePath);
    if (it != standaloneTextureCache.end())
        return it->second.textureUnit;
    return 0;
}

unsigned int TextureManager::getTextureArrayUnit(const std::string &arrayName)
{
    auto it = textureArrays.find(arrayName);
    if (it != textureArrays.end())
        return it->second.textureUnit;
    return 0;
}

int TextureManager::getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath)
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

void TextureManager::getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers)
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