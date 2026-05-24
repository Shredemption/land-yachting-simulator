#include "opengl_texture_manager.hpp"

#include "pch.h"

unsigned int OpenGLTextureManager::createTexture2D(const PendingTexture &)
{
    std::cout << "[OpenGL] createTexture2D stub\n";
    return 0;
}

unsigned int OpenGLTextureManager::createTextureArray(
    TextureArray &,
    const std::vector<PendingTexture> &)
{
    std::cout << "[OpenGL] createTextureArray stub\n";
    return 0;
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

        GLenum format = GL_RGBA;

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

void OpenGLTextureManager::uploadPending()
{
    std::cout << "[OpenGL] uploadPending stub\n";
}

void OpenGLTextureManager::clear()
{
    std::cout << "[OpenGL] clear stub\n";
}