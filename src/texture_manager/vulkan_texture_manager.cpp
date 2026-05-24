#include "vulkan_texture_manager.hpp"

#include "pch.h"

unsigned int VulkanTextureManager::createTexture2D(const PendingTexture &)
{
    std::cout << "[Vulkan] createTexture2D stub\n";
    return 0;
}

unsigned int VulkanTextureManager::createTextureArray(
    TextureArray &,
    const std::vector<PendingTexture> &)
{
    std::cout << "[Vulkan] createTextureArray stub\n";
    return 0;
}

unsigned int VulkanTextureManager::createSkybox(
    const std::vector<std::string> &)
{
    std::cout << "[Vulkan] createSkybox stub\n";
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

unsigned int VulkanTextureManager::getStandaloneTextureID(const std::string &)
{
    std::cout << "[Vulkan] getStandaloneTexture stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getStandaloneTextureUnit(const std::string &)
{
    std::cout << "[Vulkan] getStandaloneTextureUnit stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getTextureArrayUnit(const std::string &)
{
    std::cout << "[Vulkan] getTextureArrayUnit stub\n";
    return 0;
}

unsigned int VulkanTextureManager::getTextureLayerIndex(
    const std::string &,
    const std::string &)
{
    std::cout << "[Vulkan] getTextureLayerIndex stub\n";
    return 0;
}

void VulkanTextureManager::getTextureData(
    const Model &,
    unsigned int &,
    unsigned int &,
    std::vector<int> &)
{
    std::cout << "[Vulkan] getTextureData stub\n";
}