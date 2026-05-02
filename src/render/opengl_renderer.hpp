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

#include "render/render_defs.h"
#include "render/i_renderer.hpp"

enum class EngineState;
class Shader;

class OpenGLRenderer : public IRenderer
{
public:
    void renderBlankScreen() override;
    void renderLoadingScreen() override;
    void savePauseBackground() override;
    void renderMenu(EngineState state) override;

    void setup() override;
    void cleanup() override;
    void resize(int width, int height) override;

    void render() override;
    void prepareRender(::RenderBuffer &prepBuffer) override;
    void executeRender(::RenderBuffer &renderBuffer, bool toScreen = true) override;

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) override;
    renderEngine getType() const override;

private:
    void createSceneFBO(int width, int height);

    // Render functions
    void renderObjects(std::vector<RenderCommand> &renderBuffer);
    void renderModel(const RenderCommand &cmd);
    void renderHitbox(const RenderCommand &cmd);
    void renderOpaquePlane(const RenderCommand &cmd);
    void renderTransparentPlane(const RenderCommand &cmd);
    void renderGrid(const RenderCommand &cmd);
    void renderSceneSkyBox();
    void renderImage(const std::string &fileName, const glm::vec2 &position, const float width, const float height, const float alpha = 1.0f, const glm::vec2 scale = {1.0, 1.0f}, const bool uniformScaling = false, const float rotation = 0.0f, const bool mirrored = false);
    void renderSceneImages();
    void renderReflectRefract(std::vector<RenderCommand> &renderBuffer);
    void renderTestQuad(unsigned int texture, int x, int y);

    // Text
    void initFreeType();
    float calculateTextWidth(const std::string &text, float scale);
    unsigned int textVAO, textVBO;
    unsigned int textTexture;
    std::map<GLchar, Character> Characters;
    std::string fontpath = "resources/fonts/MusticaPro-SemiBold.otf";
    float textTextureSize = 128;

    FT_Library ft;
    FT_Face face;

    // Framebuffer
    unsigned int sceneTexture = 0, sceneDepthRBO = 0;
    unsigned int pauseTexture;
    unsigned int copyFBO;

    // Clipping and culling
    glm::vec4 clipPlane{0, 0, 0, 0};

    // Quad for rendering
    void initQuad();
    unsigned int quadVAO = 0, quadVBO = 0;
    float quadVertices[24] = {
        // positions   // texture coords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    // Water
    bool WaterPass = false;
    float waterHeight = 0.25;
    float waterTimer = 0.0f;

    // Debug
    glm::vec3 debugColor{1.0f, 0.1f, 0.1f};
    float FPS = 0.0f;

    // Track current and last used shader
    Shader *shader;
    Shader *lastShader = nullptr;
};