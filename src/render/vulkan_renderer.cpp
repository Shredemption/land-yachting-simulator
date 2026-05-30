#include "vulkan_renderer.hpp"

#include "pch.h"

#include <iostream>
#include <set>

#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include "ui_manager/ui_manager_defs.h"
#include "texture_manager/vulkan_texture_manager.hpp"

struct TextVertex
{
    float position[2];
    float uv[2];
    float color[4];
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to query surface capabilities");
    }

    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    std::cout << "[Vulkan]   Swapchain support: " << details.formats.size() << " formats, " << details.presentModes.size() << " present modes" << std::endl;
    return details;
}

static std::vector<const char *> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef _DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(VkDevice device) : device(device)
{
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    viewportState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    dynamicState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setShaders(VkShaderModule vert, VkShaderModule frag)
{
    shaderStages[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage =
        VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vert;
    shaderStages[0].pName = "main";

    shaderStages[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = frag;
    shaderStages[1].pName = "main";

    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setVertexInput(const VkPipelineVertexInputStateCreateInfo &info)
{
    vertexInputInfo = info;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setTopology(VkPrimitiveTopology topology)
{
    inputAssembly.topology = topology;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::enableAlphaBlending()
{
    colorBlendAttachment.blendEnable = VK_TRUE;

    colorBlendAttachment.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;

    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    colorBlendAttachment.colorBlendOp =
        VK_BLEND_OP_ADD;

    colorBlendAttachment.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE;

    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    colorBlendAttachment.alphaBlendOp =
        VK_BLEND_OP_ADD;

    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::disableBlending()
{
    colorBlendAttachment.blendEnable = VK_FALSE;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setPipelineLayout(VkPipelineLayout layout)
{
    pipelineLayout = layout;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::setRenderPass(VkRenderPass pass)
{
    renderPass = pass;
    return *this;
}

VkPipeline GraphicsPipelineBuilder::build()
{
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;

    if (vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "Failed to create graphics pipeline!");
    }

    return pipeline;
}

AllocatedBuffer VulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    AllocatedBuffer result;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(
            context.allocator,
            &bufferInfo,
            &allocInfo,
            &result.buffer,
            &result.allocation,
            nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA buffer");
    }

    return result;
}

AllocatedImage VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
{
    AllocatedImage result;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateImage(
            context.allocator,
            &imageInfo,
            &allocInfo,
            &result.image,
            &result.allocation,
            nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA image");
    }

    return result;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    bool suitable = indices.isComplete() && extensionsSupported && swapChainAdequate;
    std::cout << "[Vulkan]   Device suitable=" << suitable << " (graphics=" << indices.graphicsFamily.value_or(UINT32_MAX)
              << " present=" << indices.presentFamily.value_or(UINT32_MAX) << ")" << std::endl;
    return suitable;
}

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        const auto &queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    std::cout << "[Vulkan]   Queue families: graphics=" << indices.graphicsFamily.value_or(UINT32_MAX)
              << " present=" << indices.presentFamily.value_or(UINT32_MAX) << std::endl;
    return indices;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[Vulkan]   Failed to query device extension count" << std::endl;
        return false;
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    if (result != VK_SUCCESS)
    {
        std::cerr << "[Vulkan]   Failed to query device extension list" << std::endl;
        return false;
    }

    const char *requiredExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    bool foundRequiredExtension = false;
    for (uint32_t index = 0; index < extensionCount; ++index)
    {
        if (strcmp(availableExtensions[index].extensionName, requiredExtension) == 0)
        {
            foundRequiredExtension = true;
            break;
        }
    }

    std::cout << "[Vulkan]   Device extension support: " << (foundRequiredExtension ? "OK" : "MISSING swapchain") << std::endl;
    return foundRequiredExtension;
}

std::vector<const char *> VulkanRenderer::getDeviceExtensions()
{
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(WindowManager::window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanRenderer::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = context.physicalDevice;
    allocatorInfo.device = context.device;
    allocatorInfo.instance = instance;

    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

    if (vmaCreateAllocator(&allocatorInfo, &context.allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create VMA allocator");
    }
}

void VulkanRenderer::setup()
{
    std::cout << "[Vulkan] Setup started" << std::endl;
    try
    {
        textureManager = std::make_unique<VulkanTextureManager>(context);

        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createAllocator();

        createSwapChain();
        createImageViews();
        createRenderPass();

        createGraphicsPipeline();

        createCommandPool();
        createCommandBuffers();
        createSyncObjects();

        initTextResources();

        createFramebuffers();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Vulkan] Setup failed: " << e.what() << std::endl;
        throw;
    }
    std::cout << "[Vulkan] Setup completed" << std::endl;
}

void VulkanRenderer::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Marama";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "MaramaEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    std::cout << "[Vulkan] Creating Vulkan instance with " << extensions.size() << " extensions" << std::endl;
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
    std::cout << "[Vulkan] Vulkan instance created" << std::endl;
}

void VulkanRenderer::createSurface()
{
    std::cout << "[Vulkan] Creating window surface" << std::endl;
    if (glfwCreateWindowSurface(instance, WindowManager::window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan surface");
    }
    std::cout << "[Vulkan] Vulkan surface created" << std::endl;
}

void VulkanRenderer::pickPhysicalDevice()
{
    std::cout << "[Vulkan] Picking physical device" << std::endl;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::cout << "[Vulkan] " << deviceCount << " Vulkan-capable physical device(s) found" << std::endl;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    int bestScore = -1;
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

    for (const auto &device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        std::cout << "[Vulkan] Checking device: " << deviceProperties.deviceName << " (apiVersion="
                  << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "." << VK_VERSION_MINOR(deviceProperties.apiVersion)
                  << ")" << std::endl;

        bool suitable = isDeviceSuitable(device);
        std::cout << "[Vulkan]   isDeviceSuitable returned " << suitable << std::endl;

        if (suitable)
        {
            // Score the device based on its type (discrete GPU preferred)
            int score = 0;
            switch (deviceProperties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score = 4;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score = 3;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score = 2;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score = 1;
                break;
            default:
                score = 0;
            }

            std::cout << "[Vulkan]   Device type: " << deviceProperties.deviceType << " (score: " << score << ")" << std::endl;

            if (score > bestScore)
            {
                bestScore = score;
                bestDevice = device;
                std::cout << "[Vulkan]   New best device!" << std::endl;
            }
        }
    }

    if (bestDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    context.physicalDevice = bestDevice;
    VkPhysicalDeviceProperties bestProps;
    vkGetPhysicalDeviceProperties(context.physicalDevice, &bestProps);
    std::cout << "[Vulkan] Selected physical device: " << bestProps.deviceName << " (type: " << bestProps.deviceType << ")" << std::endl;
}

void VulkanRenderer::createLogicalDevice()
{
    std::cout << "[Vulkan] Creating logical device" << std::endl;
    QueueFamilyIndices indices = findQueueFamilies(context.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(getDeviceExtensions().size());
    createInfo.ppEnabledExtensionNames = getDeviceExtensions().data();

    if (vkCreateDevice(context.physicalDevice, &createInfo, nullptr, &context.device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(context.device, indices.graphicsFamily.value(), 0, &context.graphicsQueue);
    vkGetDeviceQueue(context.device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanRenderer::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(context.physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(context.physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    std::cout << "[Vulkan] Swap chain settings: format=" << surfaceFormat.format << ", colorSpace=" << surfaceFormat.colorSpace
              << ", presentMode=" << presentMode << ", extent=" << extent.width << "x" << extent.height << std::endl;

    if (vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    std::cout << "[Vulkan] Swap chain created" << std::endl;

    vkGetSwapchainImagesKHR(context.device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(context.device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanRenderer::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(context.device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image views!");
        }
    }
    std::cout << "[Vulkan] Created " << swapChainImageViews.size() << " image view(s)" << std::endl;
}

void VulkanRenderer::createRenderPass()
{
    std::cout << "[Vulkan] Creating render pass" << std::endl;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
    std::cout << "[Vulkan] Render pass created" << std::endl;
}

void VulkanRenderer::createGraphicsPipeline()
{
    std::cout << "[Vulkan] Creating pipeline layout" << std::endl;

    auto vertShaderCode = readFile("./resources/shaders/vulkan/vert.spv");
    auto fragShaderCode = readFile("./resources/shaders/vulkan/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    GraphicsPipelineBuilder builder(context.device);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    builder
        .setShaders(vertShaderModule, fragShaderModule)
        .setVertexInput(vertexInput)
        .disableBlending()
        .setRenderPass(renderPass)
        .setPipelineLayout(pipelineLayout);

    graphicsPipeline = builder.build();

    vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context.device, vertShaderModule, nullptr);

    std::cout << "[Vulkan] Graphics pipeline created" << std::endl;
}

void VulkanRenderer::initTextResources()
{
    std::cout << "[Vulkan] Initializing text resources" << std::endl;

    if (!ensureFontAtlas())
        throw std::runtime_error("Failed to initialize font atlas");

    createTextDescriptorSetLayout();
    initTextPipeline();
    initTextAtlasResources();
    createTextVertexBuffer();

    textResourcesInitialized = true;

    std::cout << "[Vulkan] Text resources initialized" << std::endl;
}

void VulkanRenderer::initTextPipeline()
{
    auto vertShaderCode = readFile("./resources/shaders/vulkan/text_quad.vert.spv");
    auto fragShaderCode = readFile("./resources/shaders/vulkan/text_color.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineLayoutCreateInfo layout{};
    layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout.setLayoutCount = 1;
    layout.pSetLayouts = &textDescriptorSetLayout;

    if (vkCreatePipelineLayout(context.device, &layout, nullptr, &textPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create text pipeline layout!");
    }

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(TextVertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3]{};

    attrs[0].binding = 0;
    attrs[0].location = 0;
    attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[0].offset = offsetof(TextVertex, position);

    attrs[1].binding = 0;
    attrs[1].location = 1;
    attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[1].offset = offsetof(TextVertex, uv);

    attrs[2].binding = 0;
    attrs[2].location = 2;
    attrs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attrs[2].offset = offsetof(TextVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 3;
    vertexInput.pVertexAttributeDescriptions = attrs;

    GraphicsPipelineBuilder builder(context.device);

    builder
        .setShaders(vertShaderModule, fragShaderModule)
        .setVertexInput(vertexInput)
        .enableAlphaBlending()
        .setRenderPass(renderPass)
        .setPipelineLayout(textPipelineLayout);

    textPipeline = builder.build();

    vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context.device, vertShaderModule, nullptr);

    std::cout << "[Vulkan] Text pipeline created" << std::endl;
}

void VulkanRenderer::initImageResources()
{
    std::cout << "[Vulkan] Initializing image resources" << std::endl;

    createImageDescriptorSetLayout();
    initImagePipeline();
    createImageVertexBuffer();
    initImageAtlasResources();

    imageResourcesInitialized = true;

    std::cout << "[Vulkan] Image resources initialized" << std::endl;
}

void VulkanRenderer::initImageAtlasResources()
{
    std::cout << "[Vulkan] Initializing image atlas (dummy)" << std::endl;

    // =========================================================
    // 1. CREATE DUMMY IMAGE (1x1 black)
    // =========================================================
    AllocatedImage img = createImage(
        1, 1,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    dummyImage = img.image;

    // =========================================================
    // 2. STAGING BUFFER (1 pixel)
    // =========================================================
    uint32_t pixel = 0x00000000; // RGBA black

    auto staging = context.createStagingBuffer(&pixel, sizeof(pixel));

    // =========================================================
    // 3. TRANSFER IMAGE
    // =========================================================
    transition(dummyImage,
               VK_IMAGE_LAYOUT_UNDEFINED,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(staging.buffer, dummyImage, 1, 1);

    transition(dummyImage,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // =========================================================
    // 4. IMAGE VIEW
    // =========================================================
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = dummyImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(context.device, &viewInfo, nullptr, &textAtlasImageView);

    // =========================================================
    // 5. SAMPLER
    // =========================================================
    dummySampler = context.createSampler(true);

    // =========================================================
    // 6. DESCRIPTOR SET (REUSE SAME LOGIC AS TEXT)
    // =========================================================
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &textDescriptorPool);

    VkDescriptorSetAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = textDescriptorPool;
    alloc.descriptorSetCount = 1;
    alloc.pSetLayouts = &imageDescriptorSetLayout;

    vkAllocateDescriptorSets(context.device, &alloc, &imageDescriptorSet);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textAtlasImageView;
    imageInfo.sampler = dummySampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = imageDescriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

    vmaDestroyBuffer(context.allocator, staging.buffer, staging.allocation);

    std::cout << "[Vulkan] Image atlas initialized (dummy)" << std::endl;
}

void VulkanRenderer::initImagePipeline()
{
    auto vertShaderCode = readFile("./resources/shaders/vulkan/image_quad.vert.spv");
    auto fragShaderCode = readFile("./resources/shaders/vulkan/image.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineLayoutCreateInfo layout{};
    layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout.setLayoutCount = 1;
    layout.pSetLayouts = &imageDescriptorSetLayout;

    if (vkCreatePipelineLayout(context.device, &layout, nullptr, &imagePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image pipeline layout!");
    }

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(ImageVertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3]{};

    attrs[0].binding = 0;
    attrs[0].location = 0;
    attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[0].offset = offsetof(ImageVertex, position);

    attrs[1].binding = 0;
    attrs[1].location = 1;
    attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[1].offset = offsetof(ImageVertex, uv);

    attrs[2].binding = 0;
    attrs[2].location = 2;
    attrs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attrs[2].offset = offsetof(ImageVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = 3;
    vertexInput.pVertexAttributeDescriptions = attrs;

    GraphicsPipelineBuilder builder(context.device);

    builder
        .setShaders(vertShaderModule, fragShaderModule)
        .setVertexInput(vertexInput)
        .enableAlphaBlending()
        .setRenderPass(renderPass)
        .setPipelineLayout(imagePipelineLayout);

    imagePipeline = builder.build();

    vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context.device, vertShaderModule, nullptr);
}

void VulkanRenderer::createImageVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(ImageVertex) * 100000;

    imageVertexBuffer = createBuffer(
                            bufferSize,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VMA_MEMORY_USAGE_CPU_TO_GPU)
                            .buffer;
}

void VulkanRenderer::transition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cmd;

    cmd = context.beginSingleTimeCommands();

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

    VkPipelineStageFlags srcStage = 0;
    VkPipelineStageFlags dstStage = 0;

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
        throw std::runtime_error("Unsupported image layout transition");
    }

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    context.endSingleTimeCommands(cmd);
};

void VulkanRenderer::copyBufferToImage(VkBuffer stagingBuffer, VkImage textAtlasImage, const FontAtlas atlas)
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
    region.imageExtent = {static_cast<uint32_t>(atlas.atlasWidth()), static_cast<uint32_t>(atlas.atlasHeight()), 1};

    vkCmdCopyBufferToImage(cmd, stagingBuffer, textAtlasImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    context.endSingleTimeCommands(cmd);
};

void VulkanRenderer::copyBufferToImage(VkBuffer stagingBuffer, VkImage image, uint32_t width, uint32_t height)
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

    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        cmd,
        stagingBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    context.endSingleTimeCommands(cmd);
}

void VulkanRenderer::initTextAtlasResources()
{
    const FontAtlas &atlas = getFontAtlas();

    VkDeviceSize imageSize = static_cast<VkDeviceSize>(atlas.atlasWidth()) * static_cast<VkDeviceSize>(atlas.atlasHeight()) * 4;

    // =========================================================
    // 1. CREATE STAGING BUFFER
    // =========================================================
    AllocatedBuffer staging = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *data;
    vmaMapMemory(context.allocator, staging.allocation, &data);
    memcpy(data, atlas.atlasPixels().data(), static_cast<size_t>(imageSize));
    vmaUnmapMemory(context.allocator, staging.allocation);

    // =========================================================
    // 2. CREATE IMAGE
    // =========================================================
    AllocatedImage texture = createImage(
        atlas.atlasWidth(),
        atlas.atlasHeight(),
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    textAtlasImage = texture.image;

    // =========================================================
    // 3. IMAGE LAYOUT TRANSITIONS (INLINE VERSION)
    // =========================================================
    transition(textAtlasImage,
               VK_IMAGE_LAYOUT_UNDEFINED,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(staging.buffer, textAtlasImage, atlas);

    transition(textAtlasImage,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // =========================================================
    // 4. IMAGE VIEW
    // =========================================================
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textAtlasImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(context.device, &viewInfo, nullptr, &textAtlasImageView);

    // =========================================================
    // 5. SAMPLER
    // =========================================================
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    vkCreateSampler(context.device, &samplerInfo, nullptr, &textAtlasSampler);

    // =========================================================
    // 6. DESCRIPTOR SET
    // =========================================================
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &textDescriptorPool);

    VkDescriptorSetAllocateInfo allocSet{};
    allocSet.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocSet.descriptorPool = textDescriptorPool;
    allocSet.descriptorSetCount = 1;
    allocSet.pSetLayouts = &textDescriptorSetLayout;

    vkAllocateDescriptorSets(context.device, &allocSet, &textDescriptorSet);

    VkDescriptorImageInfo imageDesc{};
    imageDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageDesc.imageView = textAtlasImageView;
    imageDesc.sampler = textAtlasSampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = textDescriptorSet;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageDesc;

    vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);

    // cleanup staging
    vmaDestroyBuffer(context.allocator, staging.buffer, staging.allocation);

    std::cout << "[Vulkan] Text atlas initilialised" << std::endl;
}

void VulkanRenderer::createTextVertexBuffer()
{
    const VkDeviceSize textVertexBufferSize = 1 << 20; // 1 MB

    AllocatedBuffer buffer = createBuffer(textVertexBufferSize,
                                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VMA_MEMORY_USAGE_CPU_TO_GPU);

    textVertexBuffer = buffer.buffer;
    textVertexBufferAllocation = buffer.allocation;
}

void VulkanRenderer::createTextDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &textDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create text descriptor set layout");
    }
}

void VulkanRenderer::createImageDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(
            context.device,
            &info,
            nullptr,
            &imageDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image descriptor set layout");
    }
}

void VulkanRenderer::cleanupTextResources()
{
    if (textPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(context.device, textPipeline, nullptr);
        textPipeline = VK_NULL_HANDLE;
    }
    if (textPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(context.device, textPipelineLayout, nullptr);
        textPipelineLayout = VK_NULL_HANDLE;
    }
    if (textDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(context.device, textDescriptorPool, nullptr);
        textDescriptorPool = VK_NULL_HANDLE;
    }
    if (textDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(context.device, textDescriptorSetLayout, nullptr);
        textDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (textAtlasSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(context.device, textAtlasSampler, nullptr);
        textAtlasSampler = VK_NULL_HANDLE;
    }
    if (textAtlasImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(context.device, textAtlasImageView, nullptr);
        textAtlasImageView = VK_NULL_HANDLE;
    }
    if (textAtlasImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(context.device, textAtlasImage, nullptr);
        textAtlasImage = VK_NULL_HANDLE;
    }
    if (textAtlasImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(context.device, textAtlasImageMemory, nullptr);
        textAtlasImageMemory = VK_NULL_HANDLE;
    }
    if (textVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(context.device, textVertexBuffer, nullptr);
        textVertexBuffer = VK_NULL_HANDLE;
    }
    if (textVertexBufferAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(context.allocator, textVertexBuffer, textVertexBufferAllocation);
    }

    textResourcesInitialized = false;
    pendingTextDraws.clear();
}

void VulkanRenderer::cleanupImageResources()
{
    if (imagePipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(context.device, imagePipeline, nullptr);
        imagePipeline = VK_NULL_HANDLE;
    }

    if (imagePipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(context.device, imagePipelineLayout, nullptr);
        imagePipelineLayout = VK_NULL_HANDLE;
    }

    if (imageDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(context.device, imageDescriptorSetLayout, nullptr);
        imageDescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (imageVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(context.device, imageVertexBuffer, nullptr);
        imageVertexBuffer = VK_NULL_HANDLE;
    }

    if (imageVertexBufferAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(context.allocator, imageVertexBuffer, imageVertexBufferAllocation);
        imageVertexBufferAllocation = VK_NULL_HANDLE;
    }

    if (dummySampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(context.device, dummySampler, nullptr);
        dummySampler = VK_NULL_HANDLE;
    }

    if (dummyImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(context.device, dummyImage, nullptr);
        dummyImage = VK_NULL_HANDLE;
    }

    imageResourcesInitialized = false;
    pendingImageDraws.clear();
}

void VulkanRenderer::createFramebuffers()
{
    std::cout << "[Vulkan] Creating framebuffers" << std::endl;
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void VulkanRenderer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(context.physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(context.device, &poolInfo, nullptr, &context.commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }

    std::cout << "[Vulkan] Command pool created" << std::endl;
}

void VulkanRenderer::createCommandBuffers()
{
    std::cout << "[Vulkan] Allocating command buffers" << std::endl;
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(context.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
    std::cout << "[Vulkan] Allocated " << commandBuffers.size() << " command buffer(s)" << std::endl;
}

void VulkanRenderer::createSyncObjects()
{
    std::cout << "[Vulkan] Creating sync objects" << std::endl;
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
    std::cout << "[Vulkan] Sync objects created" << std::endl;
}

void VulkanRenderer::cleanup()
{
    std::cout << "[Vulkan] Renderer cleanup" << std::endl;

    cleanupTextResources();
    cleanupImageResources();

    if (context.allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(context.allocator);
        context.allocator = VK_NULL_HANDLE;
    }

    // TODO: Clean up the rest of Vulkan resources
}

VkPipeline graphicsPipeline = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

VkShaderModule vertShaderModule = VK_NULL_HANDLE;
VkShaderModule fragShaderModule = VK_NULL_HANDLE;

std::vector<char> VulkanRenderer::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(context.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}

void VulkanRenderer::render()
{
    if (framebufferResized)
    {
        recreateSwapChain();
        framebufferResized = false;
    }

    int currentIndex = renderIndex.load(std::memory_order_acquire);
    auto &buffer = renderBuffers[currentIndex];

    BufferState expected = BufferState::Ready;

    bool rendered = false;

    if (buffer.state.compare_exchange_strong(
            expected, BufferState::Rendering))
    {
        try
        {
            executeRender(buffer, true);

            buffer.state.store(BufferState::Free,
                               std::memory_order_release);

            ThreadManager::renderBufferCV.notify_all();

            rendered = true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[Vulkan] Render failed: " << e.what() << std::endl;

            buffer.state.store(BufferState::Free,
                               std::memory_order_release);

            ThreadManager::renderBufferCV.notify_all();

            rendered = true;
        }

        for (int i = 0; i < 3; ++i)
        {
            if (renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Ready)
            {
                renderIndex.store(i, std::memory_order_release);
                break;
            }
        }
    }

    if (!rendered)
    {
        // If no ready buffer, prepare one inline
        for (int i = 0; i < 3; ++i)
        {
            if (renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Free)
            {
                renderBuffers[i].state.store(BufferState::Prepping, std::memory_order_release);
                prepareRender(renderBuffers[i]);
                renderBuffers[i].state.store(BufferState::Ready, std::memory_order_release);

                BufferState expected2 = BufferState::Ready;
                if (renderBuffers[i].state.compare_exchange_strong(expected2, BufferState::Rendering))
                {
                    try
                    {
                        executeRender(renderBuffers[i], true);

                        renderBuffers[i].state.store(BufferState::Free, std::memory_order_release);

                        ThreadManager::renderBufferCV.notify_all();
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "[Vulkan] Render failed: " << e.what() << std::endl;

                        renderBuffers[i].state.store(BufferState::Free, std::memory_order_release);

                        ThreadManager::renderBufferCV.notify_all();
                    }
                }

                renderIndex.store(i, std::memory_order_release);
                break;
            }
        }
    }
}

void VulkanRenderer::prepareRender(RenderBuffer &prepBuffer)
{
    // Clear and reserve size for buffer
    prepBuffer.commandBuffer.clear();

    if (!SceneManager::currentScene)
        return;

    prepBuffer.commandBuffer.reserve(
        SceneManager::currentScene->structModels.size() +
        SceneManager::currentScene->opaqueUnitPlanes.size() +
        SceneManager::currentScene->transparentUnitPlanes.size() +
        SceneManager::currentScene->grids.size());

    std::vector<std::future<RenderCommand>> futures;

    // Load Models
    for (auto &model : SceneManager::currentScene->structModels)
    {
        futures.push_back(std::async(std::launch::async, [&model]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::Model;

            cmd.shader = model.shader; 
            cmd.color = model.color;
            
            cmd.modelMatrix = model.u_model;
            cmd.normalMatrix = model.u_normal;

            g_renderer->getTextureManager()->getTextureBindings(*model.model, cmd.textureBinding, cmd.textureArrayHandle, cmd.textureLayers);

            cmd.animated = model.animated;

            if (cmd.animated)
            {
                cmd.boneTransforms = model.model->getReadBuffer();
                cmd.boneInverseOffsets = model.model->boneInverseOffsets;
            }

            float distanceFromCamera = glm::distance(glm::vec3(model.u_model[3]), Camera::getPosition());

            cmd.lod = 0;
            if (distanceFromCamera > SettingsManager::settings.video.lodDistance) cmd.lod = 1;
            if (SceneManager::engineState == EngineState::Title) cmd.lod = 0;

            if (cmd.lod >= model.model->lodMeshes.size())
                cmd.lod = static_cast<int>(std::round(model.model->lodMeshes.size())) - 1;

            cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->lodMeshes[cmd.lod], [](std::vector<MeshVariant>*) {});

            return cmd; }));

        // Hitboxes
        if (SettingsManager::settings.debug.showHitboxes)
        {
            if (model.model->hitboxMeshes.has_value() && !model.model->hitboxMeshes->empty())
            {
                futures.push_back(std::async(std::launch::async, [&model]()
                                             {
                    RenderCommand cmd;
                    
                    cmd.type = RenderType::Hitbox;

                    cmd.shader = shaderID::Hitbox; 
                    cmd.color = glm::vec3(1,0,0);
                    
                    cmd.modelMatrix = model.u_model;

                    cmd.animated = model.animated;

                    if (cmd.animated)
                    {
                        cmd.boneTransforms = model.model->getReadBuffer();
                        cmd.boneInverseOffsets = model.model->boneInverseOffsets;
                    }

                    cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->hitboxMeshes.value(), [](std::vector<MeshVariant>*) {});

                    return cmd; }));
            }
        }

        if (model.controlled)
        {
            prepBuffer.camPos = (model.u_model * model.model->getReadBuffer()[model.model->boneHierarchy["Armature_Cam"]->index]) * glm::vec4(0, 0, 0, 1);
            prepBuffer.camYaw = -atan2(model.u_model[0][1], model.u_model[1][1]);
        }
    }

    // Load opaque UnitPlanes
    for (auto &opaquePlane : SceneManager::currentScene->opaqueUnitPlanes)
    {
        futures.push_back(std::async(std::launch::async, [opaquePlane]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::OpaquePlane;

            cmd.shader = opaquePlane.shader;

            cmd.modelMatrix = opaquePlane.u_model;
            cmd.normalMatrix = opaquePlane.u_normal;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{opaquePlane.unitPlane});
            cmd.meshes = meshList;

            return cmd; }));
    }

    // Sort transparent planes back to front based on distance from the camera
    std::sort(SceneManager::currentScene->transparentUnitPlanes.begin(), SceneManager::currentScene->transparentUnitPlanes.end(), [&](const UnitPlaneData &a, const UnitPlaneData &b)
              {
                  float distA = glm::distance(Camera::getPosition(), a.position);
                  float distB = glm::distance(Camera::getPosition(), b.position);
                  return distA > distB; // Sort by distance: farthest first, closest last
              });

    // Load transparent UnitPlanes
    for (auto &transparentPlane : SceneManager::currentScene->transparentUnitPlanes)
    {
        futures.push_back(std::async(std::launch::async, [transparentPlane]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::TransparentPlane;

            cmd.shader = transparentPlane.shader;

            cmd.modelMatrix = transparentPlane.u_model;
            cmd.normalMatrix = transparentPlane.u_normal;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{transparentPlane.unitPlane});
            cmd.meshes = meshList;

            return cmd; }));
    }

    // Load grids
    for (auto &grid : SceneManager::currentScene->grids)
    {
        futures.push_back(std::async(std::launch::async, [grid]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::Grid;

            cmd.shader = grid.shader;

            cmd.modelMatrix = grid.u_model;
            cmd.normalMatrix = grid.u_normal;

            cmd.lod = grid.lod;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{grid.grid});
            cmd.meshes = meshList;

            return cmd; }));
    }

    for (auto &f : futures)
    {
        prepBuffer.commandBuffer.push_back(f.get());
    }
}

std::optional<RenderPrepResult> VulkanRenderer::executeRenderInit()
{
    vkWaitForFences(context.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        context.device,
        swapChain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return std::nullopt;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    vkResetFences(context.device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    return RenderPrepResult{imageIndex, result};
}

void VulkanRenderer::executeRenderFinal(uint32_t imageIndex, VkResult result)
{
    VkSemaphore waitSemaphores[] = {
        imageAvailableSemaphores[currentFrame]};

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSemaphore signalSemaphores[] = {
        renderFinishedSemaphores[currentFrame]};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo,
                      inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }
}

void VulkanRenderer::executeRender(RenderBuffer &renderBuffer, bool toScreen)
{
    (void)toScreen;

    auto prep = executeRenderInit();

    if (!prep.has_value())
    {
        return;
    }

    auto [imageIndex, result] = *prep;

    recordCommandBuffer(
        commandBuffers[currentFrame],
        imageIndex,
        renderBuffer);

    executeRenderFinal(imageIndex, result);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::cleanupSwapChain()
{
    std::cout << "[Vulkan] Cleaning up swap chain" << std::endl;

    for (auto framebuffer : swapChainFramebuffers)
    {
        if (framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(context.device, framebuffer, nullptr);
        }
    }

    swapChainFramebuffers.clear();

    for (auto imageView : swapChainImageViews)
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(context.device, imageView, nullptr);
        }
    }

    swapChainImageViews.clear();

    if (swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(context.device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }
}

void VulkanRenderer::recreateSwapChain()
{
    std::cout << "[Vulkan] Recreating swap chain" << std::endl;

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(WindowManager::window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(WindowManager::window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context.device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();

    vkDestroyPipeline(context.device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);

    createGraphicsPipeline();

    std::cout << "[Vulkan] Swap chain recreated" << std::endl;
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, RenderBuffer &renderBuffer)
{
    (void)renderBuffer;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor{};

    if (clearColorConfig.clearColorSet)
    {
        clearColor = clearColorConfig.clearColor;
    }
    else if (SceneManager::currentScene)
    {
        clearColor.color = {{
            SceneManager::currentScene->bgColor.r,
            SceneManager::currentScene->bgColor.g,
            SceneManager::currentScene->bgColor.b,
            1.0f,
        }};
    }
    else
    {
        clearColor.color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    }

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    renderPendingImages(commandBuffer);
    renderPendingText(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void VulkanRenderer::resize(int width, int height)
{
    std::cout << "[Vulkan] Resize requested: " << width << "x" << height << std::endl;

    if (width == 0 || height == 0)
        return;

    framebufferResized = true;
}

void VulkanRenderer::renderBlankScreen()
{
    RenderBuffer dummyBuffer;
    if (framebufferResized)
    {
        recreateSwapChain();
        framebufferResized = false;
    }
    try
    {
        executeRender(dummyBuffer, true);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Vulkan] renderBlankScreen failed: " << e.what() << std::endl;
    }
}

void VulkanRenderer::renderLoadingScreen()
{
    RenderBuffer dummyBuffer;
    if (framebufferResized)
    {
        recreateSwapChain();
        framebufferResized = false;
    }
    try
    {
        executeRender(dummyBuffer, true);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Vulkan] renderLoadingScreen failed: " << e.what() << std::endl;
    }
}

void VulkanRenderer::savePauseBackground()
{
    // TODO: Save pause background with Vulkan resources
}

void VulkanRenderer::drawPauseBackground(float darken, float darkenOffset)
{
    // TODO: implement
}

void VulkanRenderer::setClearColor(float r, float g, float b, float a)
{
    clearColorConfig.clearColor.color.float32[0] = r;
    clearColorConfig.clearColor.color.float32[1] = g;
    clearColorConfig.clearColor.color.float32[2] = b;
    clearColorConfig.clearColor.color.float32[3] = a;
    clearColorConfig.clearColorSet = true;
}

void VulkanRenderer::renderMenu(EngineState state)
{
    (void)state;

    if (framebufferResized)
    {
        recreateSwapChain();
        framebufferResized = false;
    }

    auto prep = executeRenderInit();

    if (!prep.has_value())
        return;

    auto [imageIndex, result] = *prep;

    RenderBuffer menuBuffer;
    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    clearColorConfig.clearColorSet = false;

    buildMenu(state);

    VkClearValue clearColor{};

    if (clearColorConfig.clearColorSet)
    {
        clearColor = clearColorConfig.clearColor;
    }
    else
    {
        clearColor.color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    }

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    renderPendingText(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }

    executeRenderFinal(imageIndex, result);
}

void VulkanRenderer::renderText(std::string text, float x, float y, float scale,
                                glm::vec3 color, float alpha, TextAlign textAlign)
{
    if (!textResourcesInitialized)
    {
        initTextResources();

        if (!ensureFontAtlas())
        {
            std::cerr << "[Vulkan] Cannot render text because font atlas failed to initialize" << std::endl;
            return;
        }
    }

    x *= WindowManager::screenUIScale * 2560.0f;
    y *= WindowManager::screenUIScale * 1440.0f;
    scale *= WindowManager::screenUIScale;

    TextDraw draw;
    draw.quads = buildTextQuads(text, x, y, scale, textAlign);
    draw.color = color;
    draw.alpha = alpha;

    pendingTextDraws.push_back(std::move(draw));
}

void VulkanRenderer::renderPendingText(VkCommandBuffer commandBuffer)
{
    if (pendingTextDraws.empty())
    {
        return;
    }

    if (!textResourcesInitialized || textPipeline == VK_NULL_HANDLE)
    {
        std::cout << "[Vulkan] No text pipeline initialised" << std::endl;
        return;
    }

    std::vector<TextVertex> vertices;
    vertices.reserve(pendingTextDraws.size() * 6);

    auto getNdc = [&](float px, float py)
    {
        TextVertex vertex{};
        vertex.position[0] = (px / static_cast<float>(swapChainExtent.width)) * 2.0f - 1.0f;
        vertex.position[1] = (py / static_cast<float>(swapChainExtent.height)) * 2.0f - 1.0f;
        return vertex;
    };

    for (const auto &draw : pendingTextDraws)
    {
        for (const auto &quad : draw.quads)
        {
            TextVertex v0 = getNdc(quad.position.x, quad.position.y + quad.size.y);
            TextVertex v1 = getNdc(quad.position.x + quad.size.x, quad.position.y);
            TextVertex v2 = getNdc(quad.position.x, quad.position.y);
            TextVertex v3 = getNdc(quad.position.x + quad.size.x, quad.position.y + quad.size.y);

            v0.uv[0] = quad.uv0.x;
            v0.uv[1] = quad.uv1.y;

            v1.uv[0] = quad.uv1.x;
            v1.uv[1] = quad.uv0.y;

            v2.uv[0] = quad.uv0.x;
            v2.uv[1] = quad.uv0.y;

            v3.uv[0] = quad.uv1.x;
            v3.uv[1] = quad.uv1.y;

            const float r = draw.color.r;
            const float g = draw.color.g;
            const float b = draw.color.b;
            const float a = draw.alpha;

            for (auto &v : {&v0, &v1, &v2, &v3})
            {
                v->color[0] = r;
                v->color[1] = g;
                v->color[2] = b;
                v->color[3] = a;
            }

            vertices.push_back(v0);
            vertices.push_back(v2);
            vertices.push_back(v1);

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v3);
        }
    }

    if (!vertices.empty())
    {
        VkDeviceSize bufferSize = vertices.size() * sizeof(TextVertex);
        void *data;
        vmaMapMemory(context.allocator, textVertexBufferAllocation, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vmaUnmapMemory(context.allocator, textVertexBufferAllocation);

        VkDeviceSize offsets[] = {0};
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textPipelineLayout,
                                0, 1, &textDescriptorSet,
                                0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                               &textVertexBuffer, offsets);
        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }

    pendingTextDraws.clear();
}

void VulkanRenderer::renderImage(const std::string &fileName, const glm::vec2 &position, const float width, const float height, const float alpha, const glm::vec2 scale, const bool uniformScaling, const float rotation, const bool mirrored)
{
    if (!imageResourcesInitialized)
    {
        initImageResources();
    }

    ImageDraw draw;
    draw.position = position;
    draw.width = width;
    draw.height = height;
    draw.alpha = alpha;
    draw.scale = scale;
    draw.uniformScaling = uniformScaling;
    draw.rotation = rotation;
    draw.mirrored = mirrored;

    pendingImageDraws.push_back(std::move(draw));
};

void VulkanRenderer::renderPendingImages(VkCommandBuffer commandBuffer)
{
    if (pendingImageDraws.empty())
        return;

    if (!imageResourcesInitialized || imagePipeline == VK_NULL_HANDLE)
    {
        std::cout << "[Vulkan] No image pipeline initialised" << std::endl;
        return;
    }

    std::vector<ImageVertex> vertices;
    vertices.reserve(pendingImageDraws.size() * 6);

    auto getNdc = [&](float px, float py)
    {
        ImageVertex v{};
        v.position[0] = (px / static_cast<float>(swapChainExtent.width)) * 2.0f - 1.0f;
        v.position[1] = (py / static_cast<float>(swapChainExtent.height)) * 2.0f - 1.0f;
        return v;
    };

    for (const auto &draw : pendingImageDraws)
    {
        float x = draw.position.x;
        float y = draw.position.y;

        float w = draw.width * draw.scale.x;
        float h = draw.height * draw.scale.y;

        // Quad corners (screen space)
        ImageVertex v0 = getNdc(x, y + h);
        ImageVertex v1 = getNdc(x + w, y);
        ImageVertex v2 = getNdc(x, y);
        ImageVertex v3 = getNdc(x + w, y + h);

        // UVs (full texture for now)
        v0.uv[0] = 0.0f;
        v0.uv[1] = 1.0f;
        v1.uv[0] = 1.0f;
        v1.uv[1] = 0.0f;
        v2.uv[0] = 0.0f;
        v2.uv[1] = 0.0f;
        v3.uv[0] = 1.0f;
        v3.uv[1] = 1.0f;

        float a = draw.alpha;

        for (auto *v : {&v0, &v1, &v2, &v3})
        {
            v->color[0] = 1.0f;
            v->color[1] = 1.0f;
            v->color[2] = 1.0f;
            v->color[3] = a;
        }

        // two triangles
        vertices.push_back(v0);
        vertices.push_back(v2);
        vertices.push_back(v1);

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v3);
    }

    // =========================================================
    // Upload to GPU
    // =========================================================
    VkDeviceSize bufferSize = vertices.size() * sizeof(ImageVertex);

    void *data = nullptr;
    vmaMapMemory(context.allocator, imageVertexBufferAllocation, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(context.allocator, imageVertexBufferAllocation);

    // =========================================================
    // Bind pipeline + descriptor + draw
    // =========================================================
    VkDeviceSize offsets[] = {0};

    vkCmdBindPipeline(commandBuffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      imagePipeline);

    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            imagePipelineLayout,
                            0, 1,
                            &imageDescriptorSet,
                            0, nullptr);

    vkCmdBindVertexBuffers(commandBuffer,
                           0, 1,
                           &imageVertexBuffer,
                           offsets);

    vkCmdDraw(commandBuffer,
              static_cast<uint32_t>(vertices.size()),
              1, 0, 0);

    pendingImageDraws.clear();
}

ITextureManager *VulkanRenderer::getTextureManager()
{
    return textureManager.get();
}