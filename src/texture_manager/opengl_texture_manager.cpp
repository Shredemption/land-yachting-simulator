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

unsigned int OpenGLTextureManager::createSkybox(
    const std::vector<std::string> &)
{
    std::cout << "[OpenGL] createSkybox stub\n";
    return 0;
}

void OpenGLTextureManager::uploadPending()
{
    std::cout << "[OpenGL] uploadPending stub\n";
}

void OpenGLTextureManager::clear()
{
    std::cout << "[OpenGL] clear stub\n";
}

unsigned int OpenGLTextureManager::getStandaloneTextureID(const std::string &)
{
    std::cout << "[OpenGL] getStandaloneTexture stub\n";
    return 0;
}

unsigned int OpenGLTextureManager::getStandaloneTextureUnit(const std::string &)
{
    std::cout << "[OpenGL] getStandaloneTextureUnit stub\n";
    return 0;
}

unsigned int OpenGLTextureManager::getTextureArrayUnit(const std::string &)
{
    std::cout << "[OpenGL] getTextureArrayUnit stub\n";
    return 0;
}

unsigned int OpenGLTextureManager::getTextureLayerIndex(
    const std::string &,
    const std::string &)
{
    std::cout << "[OpenGL] getTextureLayerIndex stub\n";
    return 0;
}

void OpenGLTextureManager::getTextureData(
    const Model &,
    unsigned int &,
    unsigned int &,
    std::vector<int> &)
{
    std::cout << "[OpenGL] getTextureData stub\n";
}