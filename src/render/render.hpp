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
enum class SettingsPage;

namespace Render
{
    inline std::array<::RenderBuffer, 3> renderBuffers;
    inline std::atomic<int> prepIndex = 0, renderIndex = 1, standbyIndex = 2;

    inline unsigned int sceneFBO = 0;

    inline std::vector<std::pair<std::string, float>> debugPhysicsData;

    void renderBlankScreen();
    void renderLoadingScreen();
    void savePauseBackground();

    void setup();
    void resize(int width, int height);

    void render();
    void prepareRender(::RenderBuffer &prepBuffer);
    void executeRender(::RenderBuffer &renderBuffer, bool toScreen = true);

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left);
};