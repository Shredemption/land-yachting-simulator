#include "vulkan_texture_manager.hpp"

#include "pch.h"

#include "render/vulkan_renderer.hpp"

VulkanTextureManager::VulkanTexture VulkanTextureManager::createTexture2D(const PendingTexture &tex)
{
    VulkanTexture result;

    result.width = tex.width;
    result.height = tex.height;
    result.format = VK_FORMAT_R8G8B8A8_SRGB;

    VkDeviceSize imageSize =
        static_cast<VkDeviceSize>(tex.width) *
        static_cast<VkDeviceSize>(tex.height) *
        static_cast<VkDeviceSize>(tex.channels);

    auto staging = context.createStagingBuffer(tex.pixelData.data(), imageSize);

    createImage2D(
        tex.width,
        tex.height,
        result.format,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        result.image,
        result.memory);

    transitionImageLayout(
        result.image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(
        staging.buffer,
        result.image,
        tex.width,
        tex.height);

    transitionImageLayout(
        result.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    result.view = context.createImageView(
        result.image,
        result.format);

    result.sampler = context.createSampler(tex.repeating);

    result.ready = true;

    vmaDestroyBuffer(context.allocator, staging.buffer, staging.allocation);

    return result;
}

VulkanTextureManager::VulkanTextureArray VulkanTextureManager::createTextureArray(TextureArray &array, const std::vector<PendingTexture> &layers)
{
    VulkanTextureArray result;

    result.width = array.width;
    result.height = array.height;
    result.layerCount = static_cast<uint32_t>(layers.size());
    result.format = VK_FORMAT_R8G8B8A8_SRGB;

    VkDeviceSize layerSize =
        static_cast<VkDeviceSize>(array.width) *
        static_cast<VkDeviceSize>(array.height) * 4;

    VkDeviceSize totalSize = layerSize * result.layerCount;

    auto staging = context.createStagingBuffer(nullptr, totalSize);

    void *mapped = nullptr;
    vmaMapMemory(context.allocator, staging.allocation, &mapped);

    size_t offset = 0;
    int layerIndex = 0;

    for (const auto &layer : layers)
    {
        memcpy(
            static_cast<char *>(mapped) + offset,
            layer.pixelData.data(),
            layerSize);

        result.layerMap[layer.path] = layerIndex++;

        offset += layerSize;
    }

    vmaUnmapMemory(context.allocator, staging.allocation);

    createImage2DArray(
        array.width,
        array.height,
        result.layerCount,
        result.format,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        result.image,
        result.memory);

    transitionImageLayout(
        result.image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImageArray(
        staging.buffer,
        result.image,
        array.width,
        array.height,
        result.layerCount);

    transitionImageLayout(
        result.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    result.view = context.createImageViewArray(
        result.image,
        result.format);

    result.sampler = context.createSampler(true);

    result.ready = true;

    vmaDestroyBuffer(context.allocator, staging.buffer, staging.allocation);

    return result;
}

unsigned int VulkanTextureManager::uploadSkybox(const SkyboxAsset &)
{
    std::cout << "[Vulkan] uploadSkybox stub\n";
    return 0;
}

void VulkanTextureManager::uploadPending()
{
    for (auto &[path, tex] : TextureAssetManager::standaloneTextures)
    {
        if (textures.find(path) != textures.end())
            continue;

        textures[path] = createTexture2D(tex);
    }

    for (auto &[name, arr] : TextureAssetManager::textureArrays)
    {
        if (arrays.find(name) != arrays.end())
            continue;

        arrays[name] = createTextureArray(arr, arr.layers);
    }
}

void VulkanTextureManager::clear()
{
    for (auto &[path, tex] : textures)
    {
        if (tex.view)
            vkDestroyImageView(context.device, tex.view, nullptr);

        if (tex.sampler)
            vkDestroySampler(context.device, tex.sampler, nullptr);

        if (tex.image)
            vkDestroyImage(context.device, tex.image, nullptr);

        if (tex.memory)
            vkFreeMemory(context.device, tex.memory, nullptr);
    }

    textures.clear();

    for (auto &[name, arr] : arrays)
    {
        if (arr.view)
            vkDestroyImageView(context.device, arr.view, nullptr);

        if (arr.sampler)
            vkDestroySampler(context.device, arr.sampler, nullptr);

        if (arr.image)
            vkDestroyImage(context.device, arr.image, nullptr);

        if (arr.memory)
            vkFreeMemory(context.device, arr.memory, nullptr);
    }

    arrays.clear();

    for (auto &[name, sky] : skyboxes)
    {
        if (sky.view)
            vkDestroyImageView(context.device, sky.view, nullptr);

        if (sky.sampler)
            vkDestroySampler(context.device, sky.sampler, nullptr);

        if (sky.image)
            vkDestroyImage(context.device, sky.image, nullptr);

        if (sky.memory)
            vkFreeMemory(context.device, sky.memory, nullptr);
    }

    skyboxes.clear();
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

void VulkanTextureManager::getTextureBindings(const Model &model, unsigned int &textureBinding, unsigned int &textureArrayHandle, std::vector<int> &textureLayerIndices)
{
    std::cout << "[Vulkan] getTextureBindings stub\n";
}

void VulkanTextureManager::createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(context.device, &imageInfo, nullptr, &image);

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context.findMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(context.device, &allocInfo, nullptr, &memory);

    vkBindImageMemory(context.device, image, memory, 0);
}

void VulkanTextureManager::createImage2DArray(uint32_t width, uint32_t height, uint32_t layerCount, VkFormat format, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &memory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;

    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = layerCount;

    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(context.device, &imageInfo, nullptr, &image);

    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture array image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    allocInfo.memoryTypeIndex = context.findMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(context.device, &allocInfo, nullptr, &memory);
    vkBindImageMemory(context.device, image, memory, 0);
}

void VulkanTextureManager::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cmd = context.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::runtime_error("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    context.endSingleTimeCommands(cmd);
}

void VulkanTextureManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = context.beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    context.endSingleTimeCommands(cmd);
}

void VulkanTextureManager::copyBufferToImageArray(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkCommandBuffer cmd = context.beginSingleTimeCommands();

    std::vector<VkBufferImageCopy> regions(layerCount);

    VkDeviceSize offset = 0;
    VkDeviceSize layerSize = width * height * 4;

    for (uint32_t i = 0; i < layerCount; i++)
    {
        regions[i].bufferOffset = offset;
        regions[i].bufferRowLength = 0;
        regions[i].bufferImageHeight = 0;

        regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[i].imageSubresource.mipLevel = 0;
        regions[i].imageSubresource.baseArrayLayer = i;
        regions[i].imageSubresource.layerCount = 1;

        regions[i].imageOffset = {0, 0, 0};
        regions[i].imageExtent = {width, height, 1};

        offset += layerSize;
    }

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()), regions.data());

    context.endSingleTimeCommands(cmd);
}