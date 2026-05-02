#pragma once

#include <memory>
#include "render/i_renderer.hpp"
#include "settings_manager/settings.h"

class RendererFactory
{
public:
    static std::unique_ptr<IRenderer> create();
    static bool isAvailable(renderEngine engine);
    static void checkVulkanSupport();
};