#pragma once

#include "render/base_renderer.hpp"

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
};