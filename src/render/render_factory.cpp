#include "pch.h"

#include "render_factory.hpp"
#include "opengl_renderer.hpp"
#include "vulkan_renderer.hpp"

#include <iostream>

std::unique_ptr<IRenderer> RendererFactory::create()
{
    renderEngine preferred = SettingsManager::settings.video.renderEngine;

    if (preferred == renderEngine::Vulkan)
    {
        if (!VulkanUtils::isVulkanSupported())
        {
            std::cout << "[Renderer] Vulkan unavailable: " << VulkanUtils::getVulkanError() << "\n";
            std::cout << "[Renderer] Falling back to OpenGL\n";
            SettingsManager::settings.video.renderEngine = renderEngine::OpenGL;
            SettingsManager::save();
            return std::make_unique<OpenGLRenderer>();
        }
        std::cout << "[Renderer] Vulkan: " << VulkanUtils::getVulkanError() << "\n";
        return std::make_unique<VulkanRenderer>();
    }

    std::cout << "[Renderer] Using OpenGL\n";
    return std::make_unique<OpenGLRenderer>();
}

bool RendererFactory::isAvailable(renderEngine engine)
{
    if (engine == renderEngine::OpenGL)
        return true;
    if (engine == renderEngine::Vulkan)
        return VulkanUtils::isVulkanSupported();
    return false;
}