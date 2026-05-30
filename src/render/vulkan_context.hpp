#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct StagingBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};

class VulkanContext
{
public:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;

    VmaAllocator allocator;

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer cmd);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    StagingBuffer createStagingBuffer(const void *data, VkDeviceSize size);

    VkImageView createImageView(VkImage image, VkFormat format);
    VkImageView createImageViewArray(VkImage image, VkFormat format);

    VkSampler createSampler(bool repeating);
};