#pragma once

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>

#include <memory>
#include <array>
#include <atomic>
#include <vector>
#include <map>

#include "render/renderbuffer.h"
#include "render/render_defs.h"

enum class EngineState;

class OpenGLRenderer : public IRenderer
{
public:
    std::array<::RenderBuffer, 3> renderBuffers;
    std::atomic<int> prepIndex = 0, renderIndex = 1, standbyIndex = 2;

    unsigned int sceneFBO = 0;

    std::vector<std::pair<std::string, float>> debugPhysicsData;

    void renderBlankScreen() override;
    void renderLoadingScreen() override;
    void savePauseBackground() override;
    void renderMenu(EngineState state) override;

    void setup() override;
    void resize(int width, int height) override;

    void render() override;
    void prepareRender(::RenderBuffer &prepBuffer) override;
    void executeRender(::RenderBuffer &renderBuffer, bool toScreen = true) override;

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) override;

private:
    void createSceneFBO(int width, int height);
};