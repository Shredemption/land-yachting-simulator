#include "vulkan_renderer.hpp"

#include "pch.h"

#include <iostream>
#include <set>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "ui_manager/ui_manager_defs.h"

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

void VulkanRenderer::setup()
{
    std::cout << "[Vulkan] Setup started" << std::endl;
    try
    {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        initTextResources();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
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

    physicalDevice = bestDevice;
    VkPhysicalDeviceProperties bestProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &bestProps);
    std::cout << "[Vulkan] Selected physical device: " << bestProps.deviceName << " (type: " << bestProps.deviceType << ")" << std::endl;
}

void VulkanRenderer::createLogicalDevice()
{
    std::cout << "[Vulkan] Creating logical device" << std::endl;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

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

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanRenderer::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

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

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    std::cout << "[Vulkan] Swap chain created" << std::endl;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

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

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
    std::cout << "[Vulkan] Render pass created" << std::endl;
}

void VulkanRenderer::createGraphicsPipeline()
{
    std::cout << "[Vulkan] Creating pipeline layout" << std::endl;

    auto vertShaderCode = readFile("./resources/shaders/vert.spv");
    auto fragShaderCode = readFile("./resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // No vertex buffer
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    colorBlendAttachment.blendEnable = VK_FALSE;

    VkDynamicState dynamicStates[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(
            device,
            &pipelineLayoutInfo,
            nullptr,
            &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

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

    if (vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    std::cout << "[Vulkan] Graphics pipeline created" << std::endl;
}

struct TextVertex
{
    float position[2];
    float color[4];
};

void VulkanRenderer::initTextResources()
{
    std::cout << "[Vulkan] Initializing text resources" << std::endl;

    if (!ensureFontAtlas())
    {
        throw std::runtime_error("Failed to initialize font atlas for Vulkan text rendering");
    }

    const std::string vertexShaderPath = "./resources/shaders/text_quad.vert.spv";
    const std::string fragmentShaderPath = "./resources/shaders/text_color.frag.spv";

    auto vertShaderCode = readFile(vertexShaderPath);
    auto fragShaderCode = readFile(fragmentShaderPath);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(TextVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(TextVertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(TextVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &textPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create text pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = textPipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &textPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create text graphics pipeline!");
    }

    // Allocate a host-visible vertex buffer for text quads.
    const VkDeviceSize textVertexBufferSize = 1 << 20; // 1 MB

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = textVertexBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &textVertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan text vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, textVertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &textVertexBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate text vertex buffer memory!");
    }

    vkBindBufferMemory(device, textVertexBuffer, textVertexBufferMemory, 0);

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    textResourcesInitialized = true;

    std::cout << "[Vulkan] Text resources initialized" << std::endl;
}

void VulkanRenderer::cleanupTextResources()
{
    if (textPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, textPipeline, nullptr);
        textPipeline = VK_NULL_HANDLE;
    }
    if (textPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, textPipelineLayout, nullptr);
        textPipelineLayout = VK_NULL_HANDLE;
    }
    if (textDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, textDescriptorPool, nullptr);
        textDescriptorPool = VK_NULL_HANDLE;
    }
    if (textDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, textDescriptorSetLayout, nullptr);
        textDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (textAtlasSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, textAtlasSampler, nullptr);
        textAtlasSampler = VK_NULL_HANDLE;
    }
    if (textAtlasImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, textAtlasImageView, nullptr);
        textAtlasImageView = VK_NULL_HANDLE;
    }
    if (textAtlasImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(device, textAtlasImage, nullptr);
        textAtlasImage = VK_NULL_HANDLE;
    }
    if (textAtlasImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, textAtlasImageMemory, nullptr);
        textAtlasImageMemory = VK_NULL_HANDLE;
    }
    if (textVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, textVertexBuffer, nullptr);
        textVertexBuffer = VK_NULL_HANDLE;
    }
    if (textVertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, textVertexBufferMemory, nullptr);
        textVertexBufferMemory = VK_NULL_HANDLE;
    }

    textResourcesInitialized = false;
    pendingTextDraws.clear();
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

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void VulkanRenderer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
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
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
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
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
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

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
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

            TextureManager::getTextureData(*model.model, cmd.textureUnit, cmd.textureArrayID, cmd.textureLayers);

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
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        device,
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

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

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

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
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
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    swapChainFramebuffers.clear();

    for (auto imageView : swapChainImageViews)
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }
    }

    swapChainImageViews.clear();

    if (swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
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

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

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

    if (SceneManager::currentScene)
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
        clearColor.color = {0.1, 0.1, 0.1};
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
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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
    {
        return;
    }

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

    VkClearValue clearColor{};

    float alpha = std::clamp(UIManager::fade / UIManager::fadeTime, 0.0f, 1.0f);

    switch (SceneManager::engineState)
    {
    case EngineState::Title:
    case EngineState::TitleSettings:
    case EngineState::TestMenu:
    {

        float color = 0.0f;

        if (UIManager::shouldFadeBackground)
            color = easeInOutQuad(0.0f, 0.5f, alpha);
        else
            color = 0.5f;

        clearColor.color = {color, 0, 0};
        break;
    }
    default:
        clearColor.color = {0.1, 0.1, 0.1};
    }

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    float titleX = 0.02f, titleY = 0.04f;
    float shadowDistance = 0.003f;
    std::string titleText;

    switch (state)
    {
    case EngineState::Title:
        titleText = "Land Yachting Simulator";
        break;
    case EngineState::Pause:
        titleText = "Paused";
        break;
    case EngineState::TestMenu:
        titleText = "Tests";
        break;
    case EngineState::Settings:
    case EngineState::TitleSettings:
        titleText = "Settings";
        break;
    }

    float positionOffset = easeInOutQuad(-0.01f, 0.0f, alpha);

    renderText(titleText, titleX + positionOffset + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Left);
    renderText(titleText, titleX + positionOffset, titleY, 1.0f, glm::vec3(1.0f), alpha, TextAlign::Left);

    if (UIManager::needsRestart)
    {
        renderText("Will restart to apply changes", 0.98f + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Right);
        renderText("Will restart to apply changes", 0.98f, titleY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), alpha, TextAlign::Right);
    }
    else if (UIManager::needsReload)
    {
        renderText("Will reload to apply changes", 0.98f + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Right);
        renderText("Will reload to apply changes", 0.98f, titleY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), alpha, TextAlign::Right);
    }

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

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

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
        std::cout << "[Vulkan] No text pending" << std::endl;
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
        vkMapMemory(device, textVertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), bufferSize);
        vkUnmapMemory(device, textVertexBufferMemory);

        VkDeviceSize offsets[] = {0};
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, textPipeline);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &textVertexBuffer, offsets);
        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }

    pendingTextDraws.clear();
}