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

unsigned int VulkanTextureManager::getTextureHandle(const std::string &texturePath)
{
    std::cout << "[Vulkan] getTextureHandle stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getTextureArrayHandle(const std::string &arrayName)
{
    std::cout << "[Vulkan] getTextureArrayHandle stub\n";
    return 0;
}

int VulkanTextureManager::getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath)
{
    std::cout << "[Vulkan] getTextureLayerIndex stub\n";
    return 0;
}

void VulkanTextureManager::getTextureBindings(const Model &model, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices)
{
    std::cout << "[Vulkan] getTextureBindings stub\n";
}