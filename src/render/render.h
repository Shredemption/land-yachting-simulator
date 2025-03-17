#ifndef RENDER_H
#define RENDER_H

#include "scene/scene.h"
#include "glm/glm.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
    glm::ivec2 Size;     // Size of the character
    glm::ivec2 Bearing;  // Offset from the baseline
    GLuint Advance;      // Distance to the next character
    glm::vec4 TexCoords; // (x, y, width, height)
};
class Render
{
public:
    static void initQuad();
    static void render(Scene &scene);
    static glm::vec4 clipPlane;
    static float waterHeight;

    static Texture LoadStandaloneTexture(std::string fileName);

    static void initFreeType(std::string &fontPath);
    static void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
    static FT_Library ft;
    static FT_Face face;
    static GLuint textVAO, textVBO;
    static GLuint textTexture;
    static std::map<GLchar, Character> Characters;
    static std::string fontpath;

    static bool debugMenu;
    static std::vector<std::pair<std::string, float>> debugData;

private:
    static void renderSceneModels(Scene &scene, glm::vec4 clipPlane);
    static void renderSceneUnitPlanes(Scene &scene, glm::vec4 clipPlane);
    static void renderSceneSkyBox(Scene &scene);

    static void renderModel(ModelData model);
    static void renderModel(UnitPlaneData unitPlane);

    static void renderAnimated(Mesh mesh);
    static void renderDefault(Mesh mesh);
    static void renderPBR(Mesh mesh);
    static void renderSimple(Mesh mesh);
    static void renderWater(Mesh mesh);

    static void renderReflectRefract(Scene &scene, glm::vec4 clipPlane);
    static void renderTestQuad(GLuint texture, int x, int y);

    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];
    static bool WaterPass;
};

#endif