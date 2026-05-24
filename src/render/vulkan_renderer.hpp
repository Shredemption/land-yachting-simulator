#pragma once

#include <optional>
#include <vulkan/vulkan.h>

#include "render/base_renderer.hpp"
#include "settings_manager/settings.h"
#include "texture_manager/vulkan_texture_manager.hpp"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct RenderPrepResult
{
    uint32_t imageIndex;
    VkResult result;
};

struct AllocatedBuffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct AllocatedImage
{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

class GraphicsPipelineBuilder
{
public:
    explicit GraphicsPipelineBuilder(VkDevice device);

    GraphicsPipelineBuilder &setShaders(
        VkShaderModule vert,
        VkShaderModule frag);

    GraphicsPipelineBuilder &setVertexInput(
        const VkPipelineVertexInputStateCreateInfo &info);

    GraphicsPipelineBuilder &setTopology(
        VkPrimitiveTopology topology);

    GraphicsPipelineBuilder &enableAlphaBlending();

    GraphicsPipelineBuilder &disableBlending();

    GraphicsPipelineBuilder &setPipelineLayout(
        VkPipelineLayout layout);

    GraphicsPipelineBuilder &setRenderPass(
        VkRenderPass renderPass);

    VkPipeline build();

private:
    VkDevice device;

    VkPipelineShaderStageCreateInfo shaderStages[2]{};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDynamicStateCreateInfo dynamicState{};

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkDynamicState dynamicStates[2] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
};

class VulkanRenderer : public BaseRenderer
{
public:
    void setup() override;
    void cleanup() override;
    void render() override;
    void prepareRender(RenderBuffer &prepBuffer) override;
    void executeRender(RenderBuffer &renderBuffer, bool toScreen = true) override;
    void resize(int width, int height) override;
    void renderBlankScreen() override;
    void renderLoadingScreen() override;
    void savePauseBackground() override;
    void renderMenu(EngineState state) override;
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) override;
    renderEngine getType() const override { return renderEngine::Vulkan; }

    ITextureManager *getTextureManager() override;

private:
    const int MAX_FRAMES_IN_FLIGHT = 2;

    std::unique_ptr<VulkanTextureManager> textureManager;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, RenderBuffer &renderBuffer);
    void recreateSwapChain();
    void cleanupSwapChain();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char *> getDeviceExtensions();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Vulkan text rendering resources
    void initTextResources();
    void initTextPipeline();
    void initTextAtlasResources();
    void createTextVertexBuffer();
    void cleanupTextResources();
    void createTextDescriptorSetLayout();
    void renderPendingText(VkCommandBuffer commandBuffer);

    VkImage textAtlasImage = VK_NULL_HANDLE;
    VkDeviceMemory textAtlasImageMemory = VK_NULL_HANDLE;
    VkImageView textAtlasImageView = VK_NULL_HANDLE;
    VkSampler textAtlasSampler = VK_NULL_HANDLE;
    VkBuffer textVertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory textVertexBufferMemory = VK_NULL_HANDLE;
    VkDescriptorSetLayout textDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool textDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet textDescriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout textPipelineLayout = VK_NULL_HANDLE;
    VkPipeline textPipeline = VK_NULL_HANDLE;
    bool textResourcesInitialized = false;

    AllocatedBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    AllocatedImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void copyBufferToImage(VkBuffer stagingBuffer, VkImage textAtlasImage, const FontAtlas atlas);
    void transition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    struct TextDraw
    {
        std::vector<TextQuad> quads;
        glm::vec3 color;
        float alpha;
    };

    std::vector<TextDraw> pendingTextDraws;

    std::vector<char> readFile(const std::string &filename);
    VkShaderModule createShaderModule(const std::vector<char> &code);
    std::optional<RenderPrepResult> executeRenderInit();
    void executeRenderFinal(uint32_t imageIndex, VkResult result);
};