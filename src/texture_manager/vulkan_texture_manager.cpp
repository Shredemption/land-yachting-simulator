#include "vulkan_texture_manager.hpp"

#include "pch.h"

unsigned int VulkanTextureManager::createTexture2D(const PendingTexture &)
{
    std::cout << "[Vulkan] createTexture2D stub\n";
    return 0;
}

unsigned int VulkanTextureManager::createTextureArray(TextureArray &, const std::vector<PendingTexture> &)
{
    std::cout << "[Vulkan] createTextureArray stub\n";
    return 0;
}

unsigned int VulkanTextureManager::uploadSkybox(const SkyboxAsset &)
{
    std::cout << "[Vulkan] uploadSkybox stub\n";
    return 0;
}

void VulkanTextureManager::uploadPending()
{
    std::cout << "[Vulkan] uploadPending stub\n";
}

void VulkanTextureManager::clear()
{
    std::cout << "[Vulkan] clear stub\n";
}

unsigned int VulkanTextureManager::getStandaloneTextureID(const std::string &texturePath)
{
    std::cout << "[Vulkan] getStandaloneTextureID stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getTextureArrayID(const std::string &arrayName)
{
    std::cout << "[Vulkan] getTextureArrayID stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getStandaloneTextureUnit(const std::string &texturePath)
{
    std::cout << "[Vulkan] getStandaloneTextureUnit stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getTextureArrayUnit(const std::string &arrayName)
{
    std::cout << "[Vulkan] getTextureArrayUnit stub\n";
    return 0;
}

int VulkanTextureManager::getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath)
{
    std::cout << "[Vulkan] getTextureLayerIndex stub\n";
    return 0;
}

void VulkanTextureManager::getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers)
{
    std::cout << "[Vulkan] getTextureData stub\n";
}