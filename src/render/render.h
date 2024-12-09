#ifndef RENDER_H
#define RENDER_H

#include "scene/scene.h"
#include "glm/glm.hpp"

class Render
{
public:
    static void initQuad();
    static void render(Scene& scene);
    static glm::mat4 u_view;
    static glm::mat4 u_projection;
    static glm::vec4 clipPlane;
    static float waterHeight;

private:
    static void renderSceneModels(Scene& scene, glm::vec4 clipPlane);
    static void renderSceneUnitPlanes(Scene& scene, glm::vec4 clipPlane);
    static void renderModel(Model* model);
    static void renderMesh(Mesh mesh);

    static void renderDefault(Mesh mesh);
    static void renderPBR(Mesh mesh);
    static void renderSimple(Mesh mesh);

    static void renderReflectRefract(Scene& scene, glm::vec4 clipPlane);
    static void renderTestQuad(GLuint texture, int x, int y);
    
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];
};

#endif