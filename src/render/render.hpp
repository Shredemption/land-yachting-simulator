#ifndef RENDER_HPP
#define RENDER_HPP

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
#include "scene_manager/scene_manager_defs.h"

namespace Render
{
    inline std::array<RenderBuffer, 3> renderBuffers;
    inline std::atomic<int> prepIndex = 0, renderIndex = 1, standbyIndex = 2;

    inline float waterHeight = 0.25;

    inline debugState debugstate;
    inline float FPS = 0.0f;
    inline std::vector<std::pair<std::string, float>> debugPhysicsData;

    inline glm::vec4 clipPlane(0, 0, 0, 0);
    inline float lodDistance = 30.0f;

    inline FT_Library ft;
    inline FT_Face face;

    inline unsigned int textVAO, textVBO;
    inline unsigned int textTexture;
    inline std::map<GLchar, Character> Characters;
    inline std::string fontpath = "resources/fonts/MusticaPro-SemiBold.otf";

    inline unsigned int sceneFBO = 0, sceneTexture = 0, sceneDepthRBO = 0;

    inline unsigned int pauseTexture;
    inline unsigned int copyFBO;

    void createSceneFBO(int width, int height);
    void resize(int width, int height);
    void savePauseBackground();

    void renderBlankScreen();
    void renderLoadingScreen();
    void renderTitleScreen();
    void renderMenuScreen(const EngineState &state);

    void setup();
    void initQuad();
    void render();
    void prepareRender(RenderBuffer &prepBuffer);
    void executeRender(RenderBuffer &renderBuffer, bool toScreen = true);

    void initFreeType();
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f);
};

#endif