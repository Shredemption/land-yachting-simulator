#ifndef RENDER_H
#define RENDER_H

#include "glm/glm.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "scene/scene.h"

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
    static float waterHeight;

    static bool debugMenu;
    static std::vector<std::pair<std::string, float>> debugData;

    static glm::vec4 clipPlane;

    static FT_Library ft;
    static FT_Face face;

    static GLuint textVAO, textVBO;
    static GLuint textTexture;
    static std::map<GLchar, Character> Characters;
    static std::string fontpath;

    static void setup();
    static void initQuad();
    static void render(Scene &scene);

    static Texture LoadStandaloneTexture(std::string fileName);

    static void initFreeType();
    static void renderText(std::string text, float x, float y, float scale, glm::vec3 color);

private:
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];

    static bool WaterPass;

    // Class renderers
    static void renderSceneModels(Scene &scene, glm::vec4 clipPlane);
    static void renderSceneUnitPlanes(Scene &scene, glm::vec4 clipPlane);
    static void renderSceneGrids(Scene &scene, glm::vec4 clipPlane);
    static void renderSceneSkyBox(Scene &scene);
    static void renderSceneTexts(Scene &scene);

    // Type renderers
    static void renderModel(ModelData model);
    static void renderModel(UnitPlaneData unitPlane);
    static void renderModel(GridData grid);

    // Shader renderers
    static void renderDefault(Mesh mesh);
    static void renderToon(Mesh mesh);
    static void renderToonTerrain(Mesh mesh);
    static void renderPBR(Mesh mesh);
    static void renderSimple(Mesh mesh);
    static void renderToonWater(Mesh mesh);
    static void renderWater(Mesh mesh);

    // Texture renderers
    static void renderReflectRefract(Scene &scene, glm::vec4 clipPlane);
    static void renderTestQuad(GLuint texture, int x, int y);
};

#endif