#include "opengl_texture_manager.hpp"

#include "pch.h"

unsigned int OpenGLTextureManager::createTexture2D(const PendingTexture &pending)
{
    GLuint texID;

    glActiveTexture(GL_TEXTURE0 + pending.textureUnit);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = GL_RGBA;

    switch (pending.channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
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

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

unsigned int OpenGLTextureManager::createTextureArray(TextureArray &arr, const std::vector<PendingTexture> &textures)
{
    if (textures.empty())
        return 0;

    arr.width = textures[0].width;
    arr.height = textures[0].height;

    GLuint texID;

    glActiveTexture(GL_TEXTURE0 + arr.textureUnit);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, arr.width, arr.height, (GLsizei)textures.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    for (size_t layer = 0; layer < textures.size(); ++layer)
    {
        const PendingTexture &tex = textures[layer];

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)layer, arr.width, arr.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex.pixelData.data());
        arr.textureLayerMap[tex.path] = (int)layer;
    }

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    arr.textureArrayID = texID;
    return texID;
}

unsigned int OpenGLTextureManager::uploadSkybox(const SkyboxCPU &skybox)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (size_t i = 0; i < 6; i++)
    {
        const PendingTexture &face = skybox.faces[i];

        if (face.pixelData.empty())
        {
            std::cerr << "Missing skybox face data at index " << i << "\n";
            continue;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGBA,
            face.width,
            face.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            face.pixelData.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

void OpenGLTextureManager::uploadStandalones()
{
    std::lock_guard<std::mutex> lock(TextureAssetManager::textureQueueMutex);

    while (!TextureAssetManager::textureQueue.empty())
    {
        PendingTexture &pending = TextureAssetManager::textureQueue.front();
        TextureAssetManager::textureQueue.pop();

        GLuint texID = OpenGLTextureManager::createTexture2D(pending);

        Texture tex;
        tex.path = pending.path;
        tex.index = texID;
        tex.textureUnit = pending.textureUnit;

        std::cout
            << "[CACHE] Loaded texture: "
            << pending.path
            << " -> ID=" << texID
            << " unit=" << pending.textureUnit
            << "\n";

        {
            std::lock_guard<std::mutex> cacheLock(TextureAssetManager::standaloneCacheMutex);

            TextureAssetManager::standaloneTextureCache[pending.path] = tex;
        }

        {
            std::lock_guard<std::mutex> pendingLock(
                TextureAssetManager::pendingTexturesMutex);

            TextureAssetManager::pendingTextures.erase(pending.path);
        }
    }
}

void OpenGLTextureManager::uploadTextureArrays()
{
    std::lock_guard<std::mutex> lock(TextureAssetManager::openglMutex);

    for (auto &[arrayName, arr] : TextureAssetManager::textureArrays)
    {
        if (arr.textureArrayID != 0)
            continue;

        if (arr.pendingTextures.empty())
            continue;

        createTextureArray(arr, arr.pendingTextures);

        for (auto &tex : arr.pendingTextures)
        {
            tex.pixelData.clear();
            tex.pixelData.shrink_to_fit();
        }
    }
}

void OpenGLTextureManager::uploadPending()
{
    uploadStandalones();
    uploadTextureArrays();
}

void clearStandaloneCache()
{
    std::lock_guard<std::mutex> lock(TextureAssetManager::standaloneCacheMutex);
    for (auto &pair : TextureAssetManager::standaloneTextureCache)
    {
        glDeleteTextures(1, &pair.second.index);
    }
    TextureAssetManager::standaloneTextureCache.clear();
}

void clearTextureArrays()
{
    for (auto &[name, texArray] : TextureAssetManager::textureArrays)
    {
        if (texArray.textureArrayID != 0)
        {
            glDeleteTextures(1, &texArray.textureArrayID);
            texArray.textureArrayID = 0;
        }
        texArray.textureLayerMap.clear();
        texArray.pendingTextures.clear();
    }
    TextureAssetManager::textureArrays.clear();
}

void OpenGLTextureManager::clear()
{
    clearStandaloneCache();
    clearTextureArrays();
    TextureAssetManager::nextFreeUnit = 5;
}