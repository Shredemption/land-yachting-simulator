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

unsigned int VulkanTextureManager::uploadSkybox(const SkyboxCPU &)
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