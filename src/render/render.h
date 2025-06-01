#ifndef RENDER_H
#define RENDER_H

#include "glm/glm.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

#include <memory>

#include "scene/scene.h"
#include "shader/shader.h"

struct Character
{
    glm::ivec2 Size;     // Size of the character
    glm::ivec2 Bearing;  // Offset from the baseline
    GLuint Advance;      // Distance to the next character
    glm::vec4 TexCoords; // (x, y, width, height)
};

enum debugState
{
    dbNone,
    dbFPS,
    dbPhysics
};

enum RenderType
{
    rtModel,
    rtOpaquePlane,
    rtTransparentPlane,
    rtGrid
};

struct RenderCommand
{
    RenderType type;

    shaderID shader;
    std::shared_ptr<std::vector<MeshVariant>> meshes;

    unsigned int textureUnit;
    unsigned int textureArrayID;
    std::vector<int> textureLayers;

    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;

    int lod;

    bool animated = false;
    const std::vector<glm::mat4> *boneTransforms = nullptr;
    const std::vector<glm::mat4> *boneInverseOffsets = nullptr;
};

class Render
{
public:
    static std::vector<RenderCommand> prepBuffer;
    static std::vector<RenderCommand> renderBuffer;

    static float waterHeight;

    static debugState debugState;
    static float FPS;
    static std::vector<std::pair<std::string, float>> debugPhysicsData;

    static glm::vec4 clipPlane;
    static float lodDistance;

    static FT_Library ft;
    static FT_Face face;

    static GLuint textVAO, textVBO;
    static GLuint textTexture;
    static std::map<GLchar, Character> Characters;
    static std::string fontpath;

    static void setup();
    static void initQuad();
    static void prepareRender();
    static void executeRender();

    static void initFreeType();
    static void renderText(std::string text, float x, float y, float scale, glm::vec3 color);

private:
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];

    static bool WaterPass;

    // Class renderers
    static void renderObjects();
    static void renderModel(const RenderCommand &cmd);
    static void renderOpaquePlane(const RenderCommand &cmd);
    static void renderTransparentPlane(const RenderCommand &cmd);
    static void renderGrid(const RenderCommand &cmd);
    static void renderSceneSkyBox();
    static void renderSceneTexts();
    static void renderSceneImages();

    // Texture renderers
    static void renderReflectRefract();
    static void renderTestQuad(GLuint texture, int x, int y);
};

#endif