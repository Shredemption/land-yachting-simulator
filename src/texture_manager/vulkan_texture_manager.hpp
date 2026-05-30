#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "i_texture_manager.hpp"

struct VulkanContext;

class VulkanTextureManager : public ITextureManager
{
public:
    VulkanTextureManager(VulkanContext &ctx) : context(ctx) {};
    void uploadPending() override;
    unsigned int uploadSkybox(const SkyboxAsset &skybox) override;

    void clear() override;

    unsigned int getTextureHandle(const std::string &texturePath) override;
    unsigned int getTextureArrayHandle(const std::string &arrayName) override;
    int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath) override;
    void getTextureBindings(const Model &model, unsigned int &textureBinding, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices) override;

private:
    VulkanContext &context;

    struct VulkanTexture
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        VkFormat format;
        uint32_t width = 0;
        uint32_t height = 0;

        bool ready = false;
    };

    struct VulkanTextureArray
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        VkFormat format;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t layerCount = 0;

        std::unordered_map<std::string, int> layerMap;

        bool ready = false;
    };

    struct VulkanSkybox
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        VkFormat format;

        bool ready = false;
    };

    std::unordered_map<std::string, VulkanTexture> textures;
    std::unordered_map<std::string, VulkanTextureArray> arrays;
    std::unordered_map<std::string, VulkanSkybox> skyboxes;

    VulkanTexture createTexture2D(const PendingTexture &tex);
    VulkanTextureArray createTextureArray(TextureArray &, const std::vector<PendingTexture> &layers);

    void createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory);
    void createImage2DArray(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory);

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void copyBufferToImageArray(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
};