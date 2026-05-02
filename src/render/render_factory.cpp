#include "pch.h"

#include "render_factory.hpp"
#include "opengl_renderer.hpp"
#include "vulkan_renderer.hpp"

#include <iostream>

std::unique_ptr<IRenderer> RendererFactory::create()
{
    renderEngine engine = SettingsManager::settings.video.renderEngine;

    if (engine == renderEngine::Vulkan)
    {
        std::cout << "[Renderer] Creating VulkanRenderer\n";
        return std::make_unique<VulkanRenderer>();
    }

    std::cout << "[Renderer] Creating OpenGLRenderer\n";
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

void RendererFactory::checkVulkanSupport()
{
    if (SettingsManager::settings.video.renderEngine == renderEngine::Vulkan)
    {
        if (!VulkanUtils::isVulkanSupported())
        {
            std::cout << "[Vulkan] Not supported: " << VulkanUtils::getVulkanError() << "\n";
            std::cout << "[RendererFactory] Falling back to OpenGL\n";
            SettingsManager::settings.video.renderEngine = renderEngine::OpenGL;
            SettingsManager::save();
        }
        else
        {
            std::cout << "[Vulkan] Supported\n";
        }
    }
}