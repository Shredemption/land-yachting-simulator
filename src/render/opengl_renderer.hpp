#pragma once

#include "render/i_renderer.hpp"
#include "render/render.hpp"

class OpenGLRenderer : public IRenderer
{
public:
    void setup() override { Render::setup(); }
    void cleanup() override {}
    void render() override { Render::render(); }
    void prepareRender(RenderBuffer& prepBuffer) override { Render::prepareRender(prepBuffer); }
    void executeRender(RenderBuffer& renderBuffer, bool toScreen = true) override { Render::executeRender(renderBuffer, toScreen); }
    void resize(int width, int height) override { Render::resize(width, height); }
    void renderBlankScreen() override { Render::renderBlankScreen(); }
    void renderLoadingScreen() override { Render::renderLoadingScreen(); }
    void savePauseBackground() override { Render::savePauseBackground(); }
    void renderMenu(EngineState state) override { Render::renderMenu(state); }
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) override { Render::renderText(text, x, y, scale, color, alpha, textAlign); }
    renderEngine getType() const override { return renderEngine::OpenGL; }
};