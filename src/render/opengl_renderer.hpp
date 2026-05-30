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
#include "render/base_renderer.hpp"
#include "texture_manager/opengl_texture_manager.hpp"

enum class EngineState;
class Shader;

class OpenGLRenderer : public BaseRenderer
{
public:
    void renderBlankScreen() override;
    void renderLoadingScreen() override;
    void savePauseBackground() override;
    void drawPauseBackground(float darken, float darkenOffset) override;
    void setClearColor(float r, float g, float b, float a) override;

    void setup() override;
    void cleanup() override;
    void resize(int width, int height) override;

    void render() override;
    void prepareRender(::RenderBuffer &prepBuffer) override;
    void executeRender(::RenderBuffer &renderBuffer, bool toScreen = true) override;
    void renderMenu(EngineState state) override;

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f, TextAlign textAlign = TextAlign::Left) override;
    renderEngine getType() const override;

    ITextureManager *getTextureManager() override;

private:
    std::unique_ptr<OpenGLTextureManager> textureManager;

    void createSceneFBO(int width, int height);

    // Render functions
    void renderObjects(std::vector<RenderCommand> &renderBuffer);
    void renderModel(const RenderCommand &cmd);
    void renderHitbox(const RenderCommand &cmd);
    void renderOpaquePlane(const RenderCommand &cmd);
    void renderTransparentPlane(const RenderCommand &cmd);
    void renderGrid(const RenderCommand &cmd);
    void renderSceneSkyBox();
    void renderImage(const std::string &fileName, const glm::vec2 &position, const float width, const float height, const float alpha = 1.0f, const glm::vec2 scale = {1.0, 1.0f}, const bool uniformScaling = false, const float rotation = 0.0f, const bool mirrored = false) override;
    void renderSceneImages();
    void renderReflectRefract(std::vector<RenderCommand> &renderBuffer);
    void renderTestQuad(unsigned int texture, int x, int y);

    // Text
    void initTextResources();
    unsigned int textVAO = 0, textVBO = 0;
    unsigned int textTexture = 0;

    // Framebuffer
    unsigned int sceneTexture = 0, sceneDepthRBO = 0;
    unsigned int pauseTexture = 0;
    unsigned int copyFBO = 0;

    // Clipping and culling
    glm::vec4 clipPlane{0, 0, 0, 0};

    // Quad for rendering
    void initQuad();
    unsigned int quadVAO = 0, quadVBO = 0;

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