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

enum class debugState;
struct RenderCommand;
struct RenderBuffer;
struct Character;

class Render
{
public:
    static std::array<RenderBuffer, 3> renderBuffers;
    static std::atomic<int> prepIndex;
    static std::atomic<int> renderIndex;
    static std::atomic<int> standbyIndex;

    static float waterHeight;

    static debugState debugState;
    static float FPS;
    static std::vector<std::pair<std::string, float>> debugPhysicsData;

    static glm::vec4 clipPlane;
    static float lodDistance;

    static FT_Library ft;
    static FT_Face face;

    static unsigned int textVAO, textVBO;
    static unsigned int textTexture;
    static std::map<GLchar, Character> Characters;
    static std::string fontpath;

    static unsigned int sceneFBO;
    static unsigned int sceneTexture;
    static unsigned int sceneDepthRBO;

    static unsigned int pauseTexture;
    static unsigned int copyFBO;

    static void createSceneFBO(int width, int height);
    static void resize(int width, int height);
    static void savePauseBackground();

    static void renderLoadingScreen();
    static void renderTitleScreen();
    static void renderPauseScreen();

    static void setup();
    static void initQuad();
    static void render();
    static void prepareRender(RenderBuffer &prepBuffer);
    static void executeRender(RenderBuffer &renderBuffer, bool toScreen = true);

    static void initFreeType();
    static void renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha = 1.0f);

private:
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];

    static bool WaterPass;

    // Class renderers
    static void renderObjects(std::vector<RenderCommand> &renderBuffer);
    static void renderModel(const RenderCommand &cmd);
    static void renderOpaquePlane(const RenderCommand &cmd);
    static void renderTransparentPlane(const RenderCommand &cmd);
    static void renderGrid(const RenderCommand &cmd);
    static void renderSceneSkyBox();
    static void renderSceneTexts();
    static void renderSceneImages();

    // Texture renderers
    static void renderReflectRefract(std::vector<RenderCommand> &renderBuffer);
    static void renderTestQuad(unsigned int texture, int x, int y);
};

#endif