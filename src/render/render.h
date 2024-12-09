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

private:
    static void renderSceneModels(Scene& scene);
    static void renderSceneUnitPlanes(Scene& scene);
    static void renderModel(Model* model);
    static void renderMesh(Mesh mesh);

    static void renderDefault(Mesh mesh);
    static void renderSimple(Mesh mesh);

    static void renderReflectRefract(Scene& scene);
    static void renderTestQuad(GLuint texture);
    
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static float quadVertices[];
};

#endif