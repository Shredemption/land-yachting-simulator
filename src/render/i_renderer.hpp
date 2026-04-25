#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>
#include <string>

struct RenderBuffer;
enum class EngineState;
enum class TextAlign;
enum class renderEngine;

#include "render/render_defs.h"

class Engine
{
public:
    std::unique_ptr<IRenderer> renderer;
};

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void setup() = 0;
    virtual void cleanup() = 0;
    virtual void render() = 0;
    virtual void prepareRender(RenderBuffer &prepBuffer) = 0;
    virtual void executeRender(RenderBuffer &renderBuffer, bool toScreen = true) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void renderBlankScreen() = 0;
    virtual void renderLoadingScreen() = 0;
    virtual void savePauseBackground() = 0;
    virtual void renderMenu(EngineState state) = 0;
    virtual void renderText(std::string text, float x, float y, float scale,
                            glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) = 0;
    virtual renderEngine getType() const = 0;
};

namespace VulkanUtils
{
    bool isVulkanSupported();
    const char *getVulkanError();
}