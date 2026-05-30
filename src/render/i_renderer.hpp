#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>
#include <string>
#include <array>

enum class EngineState;
enum class TextAlign;
enum class renderEngine;

#include "render/render_defs.h"
#include "texture_manager/i_texture_manager.hpp"

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    std::array<::RenderBuffer, 3> renderBuffers;
    std::atomic<int> prepIndex = 0, renderIndex = 1, standbyIndex = 2;

    unsigned int sceneFBO = 0;

    std::vector<std::pair<std::string, float>> debugPhysicsData;

    virtual void setup() = 0;
    virtual void cleanup() = 0;
    virtual void render() = 0;
    virtual void setClearColor(float r, float g, float b, float a) = 0;
    virtual void prepareRender(RenderBuffer &prepBuffer) = 0;
    virtual void executeRender(RenderBuffer &renderBuffer, bool toScreen = true) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void renderBlankScreen() = 0;
    virtual void renderLoadingScreen() = 0;
    virtual void savePauseBackground() = 0;
    virtual void drawPauseBackground(float darken, float darkenOffset) = 0;
    virtual void renderMenu(EngineState state) = 0;
    virtual void renderText(std::string text, float x, float y, float scale,
                            glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) = 0;
    virtual void renderImage(const std::string &fileName, const glm::vec2 &position, const float width, const float height,
                             const float alpha = 1.0f, const glm::vec2 scale = {1.0, 1.0f}, const bool uniformScaling = false, const float rotation = 0.0f, const bool mirrored = false) = 0;

    virtual renderEngine getType() const = 0;

    virtual ITextureManager *getTextureManager() = 0;
};

class Engine
{
public:
    std::unique_ptr<IRenderer> renderer;
};

namespace VulkanUtils
{
    bool isVulkanSupported();
    const char *getVulkanError();
}