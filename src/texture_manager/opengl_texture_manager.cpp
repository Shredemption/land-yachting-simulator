#include "opengl_texture_manager.hpp"

#include "pch.h"

int OpenGLTextureManager::allocateUnit()
{
    return nextReservedUnit++;
}

GLenum OpenGLTextureManager::formatFromChannels(int channels)
{
    switch (channels)
    {
    case 1:
        return GL_RED;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        return GL_RGBA;
    }
}

unsigned int OpenGLTextureManager::createTexture2D(const PendingTexture &tex)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto &entry = gpuTextures[tex.path];

    if (entry.ready)
        return entry.id;

    GLenum format = formatFromChannels(tex.channels);

    GLenum internalFormat = GL_RGBA8;

    switch (tex.channels)
    {
    case 1:
        internalFormat = GL_R8;
        break;
    case 3:
        internalFormat = GL_RGB8;
        break;
    case 4:
        internalFormat = GL_RGBA8;
        break;
    }

    glGenTextures(1, &entry.id);

    entry.unit = allocateUnit();

    glActiveTexture(GL_TEXTURE0 + entry.unit);
    glBindTexture(GL_TEXTURE_2D, entry.id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        tex.width,
        tex.height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        tex.pixelData.data());

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.repeating ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Keep the texture bound to its assigned unit so shaders can reference its unit persistently
    glActiveTexture(GL_TEXTURE0 + entry.unit);
    glBindTexture(GL_TEXTURE_2D, entry.id);

    entry.ready = true;

    return entry.id;
}

unsigned int OpenGLTextureManager::createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (array.ready)
        return array.textureArrayHandle;

    if (layers.empty())
        return 0;

    int width = layers[0].width;
    int height = layers[0].height;

    // If caller already assigned a texture unit (reserved), use it; otherwise allocate one
    if (array.bindingSlot == -1)
        array.bindingSlot = allocateUnit();

    glGenTextures(1, &array.textureArrayHandle);
    glActiveTexture(GL_TEXTURE0 + array.bindingSlot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, array.textureArrayHandle);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_RGBA8,
        width,
        height,
        (int)layers.size(),
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr);

    for (int i = 0; i < layers.size(); i++)
    {
        const auto &tex = layers[i];

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0,
            0, 0, i,
            tex.width,
            tex.height,
            1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            tex.pixelData.data());

        array.textureLayerMap[tex.path] = i;
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Keep the texture array bound to its assigned unit so shaders can sample it without per-frame rebinding
    glActiveTexture(GL_TEXTURE0 + array.bindingSlot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, array.textureArrayHandle);

    array.ready = true;

    return array.textureArrayHandle;
}

unsigned int OpenGLTextureManager::uploadSkybox(const SkyboxAsset &skybox)
{
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < 6; i++)
    {
        const PendingTexture &tex = skybox.faces[i];

        if (tex.pixelData.empty())
        {
            std::cerr << "Skybox face missing pixel data\n";
            continue;
        }

        GLenum format = GL_RGBA;

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            format,
            tex.width,
            tex.height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            tex.pixelData.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    return id;
}

void OpenGLTextureManager::uploadPending()
{
    auto &standalones = TextureAssetManager::getStandaloneTextures();

    for (auto it = standalones.begin(); it != standalones.end(); ++it)
    {
        createTexture2D(it->second);
    }

    auto &arrays = TextureAssetManager::getTextureArrays();

    for (auto it = arrays.begin(); it != arrays.end(); ++it)
    {
        TextureArray &arr = it->second;

        // Reserve deterministic units for texture arrays so they remain bound after upload
        if (arr.bindingSlot == -1)
        {
            arr.bindingSlot = nextReservedUnit++;
        }

        createTextureArray(arr, arr.layers);

        arrayGPUMap[it->first] = &arr;
    }
}

void OpenGLTextureManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex);

    for (auto &[k, v] : skyboxCache)
        glDeleteTextures(1, &v.id);

    gpuTextures.clear();
    skyboxCache.clear();

    nextReservedUnit = reservedBase;
}

unsigned int OpenGLTextureManager::getTextureHandle(const std::string &texturePath)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = gpuTextures.find(texturePath);
    if (it != gpuTextures.end() && it->second.ready)
        return it->second.id;

    return 0;
}

unsigned int OpenGLTextureManager::getStandaloneTextureUnit(const std::string &texturePath)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = gpuTextures.find(texturePath);
    if (it != gpuTextures.end() && it->second.ready)
        return it->second.unit;

    return 0;
}

unsigned int OpenGLTextureManager::getTextureArrayHandle(const std::string &arrayName)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = arrayGPUMap.find(arrayName);
    if (it != arrayGPUMap.end())
        return it->second->textureArrayHandle;

    return 0;
}

unsigned int OpenGLTextureManager::getTextureArrayUnit(const std::string &arrayName)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = arrayGPUMap.find(arrayName);
    if (it != arrayGPUMap.end() && it->second)
        return it->second->bindingSlot;

    return 0;
}

int OpenGLTextureManager::getSkyboxUnit()
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = skyboxCache.find("__skybox__");
    if (it != skyboxCache.end())
        return it->second.unit;
    return -1;
}

int OpenGLTextureManager::getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath)
{
    auto it = TextureAssetManager::getTextureArrays().find(arrayName);
    if (it == TextureAssetManager::getTextureArrays().end())
        return -1;

    const TextureArray &arr = it->second;

    auto layerIt = arr.textureLayerMap.find(texturePath);
    if (layerIt != arr.textureLayerMap.end())
        return layerIt->second;

    return -1;
}

void OpenGLTextureManager::getTextureBindings(const Model &model, unsigned int &textureBinding, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices)
{
    textureLayerIndices.clear();

    auto &arrays = TextureAssetManager::getTextureArrays();

    auto it = arrays.find(model.textureArrayName);
    if (it == arrays.end())
    {
        textureBinding = 0;
        textureArrayHandle = 0;
        return;
    }

    const TextureArray &arr = it->second;

    textureBinding = arr.bindingSlot;
    textureArrayHandle = arr.textureArrayHandle;

    for (const auto &path : model.texturePaths)
    {
        int layer = getTextureLayerIndex(model.textureArrayName, path);
        textureLayerIndices.push_back(layer);
    }
}