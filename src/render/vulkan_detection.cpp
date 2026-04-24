#include "i_renderer.hpp"

#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace VulkanUtils
{
    static std::string vulkanErrorMessage;

    bool isVulkanSupported()
    {
#ifdef _WIN32
        HMODULE vulkanLib = LoadLibraryA("vulkan-1.dll");
        if (!vulkanLib)
        {
            vulkanErrorMessage = "Vulkan library not found (vulkan-1.dll)";
            return false;
        }

        // Try to get vkEnumerateInstanceVersion (Vulkan 1.1+)
        // Define our own function pointer type to avoid Vulkan headers
        typedef uint32_t(*PFN_vkEnumerateInstanceVersion)();
        PFN_vkEnumerateInstanceVersion fn = (PFN_vkEnumerateInstanceVersion)GetProcAddress(vulkanLib, "vkEnumerateInstanceVersion");
        FreeLibrary(vulkanLib);

        if (fn)
        {
            uint32_t version = fn();
            uint32_t major = (version >> 22) & 0x3FF;
            uint32_t minor = (version >> 12) & 0x3FF;
            std::ostringstream oss;
            oss << "Vulkan " << major << "." << minor;
            vulkanErrorMessage = oss.str();
            return true;
        }

        vulkanErrorMessage = "Vulkan 1.0+ available";
        return true;
#else
        void* vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW);
        if (!vulkanLib)
        {
            vulkanErrorMessage = "Vulkan library not found";
            return false;
        }
        dlclose(vulkanLib);
        vulkanErrorMessage = "Vulkan available";
        return true;
#endif
    }

    const char* getVulkanError()
    {
        return vulkanErrorMessage.c_str();
    }
}