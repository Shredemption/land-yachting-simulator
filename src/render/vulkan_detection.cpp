#include "i_renderer.hpp"

#include <vulkan/vulkan.h>
#include <sstream>
#include <vector>

namespace VulkanUtils
{
    static std::string vulkanErrorMessage;

    bool isVulkanSupported()
    {
        // 1. Create Vulkan instance
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VulkanCheck";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "NoEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkInstance instance;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS)
        {
            vulkanErrorMessage = "Failed to create Vulkan instance (no driver?)";
            return false;
        }

        // 2. Check for physical devices (GPUs)
        uint32_t deviceCount = 0;
        result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (result != VK_SUCCESS || deviceCount == 0)
        {
            vkDestroyInstance(instance, nullptr);
            vulkanErrorMessage = "No Vulkan-compatible GPU found";
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // 3. Get Vulkan version (if supported)
        uint32_t version = VK_API_VERSION_1_0;

        auto fn = (PFN_vkEnumerateInstanceVersion)
            vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");

        if (fn)
        {
            fn(&version);
        }

        uint32_t major = VK_VERSION_MAJOR(version);
        uint32_t minor = VK_VERSION_MINOR(version);

        std::ostringstream oss;
        oss << "Vulkan " << major << "." << minor
            << " (" << deviceCount << " device(s))";

        vulkanErrorMessage = oss.str();

        vkDestroyInstance(instance, nullptr);
        return true;
    }

    const char *getVulkanError()
    {
        return vulkanErrorMessage.c_str();
    }
}