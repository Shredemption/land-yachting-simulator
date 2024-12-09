#include "render/render.h"
#include "event_handler/event_handler.h"
#include "frame_buffer/frame_buffer.h"

glm::mat4 Render::u_view = glm::mat4(1.0f);
glm::mat4 Render::u_projection = glm::mat4(1.0f);

unsigned int Render::quadVAO = 0, Render::quadVBO = 0;
float Render::quadVertices[] = {0};

void Render::initQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions   // texture coords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f};

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glBindVertexArray(0);
    }
}

void Render::render(Scene &scene)
{
    renderReflectRefract(scene);
    renderSceneModels(scene);
    renderSceneUnitPlanes(scene);
    // renderTestQuad(FrameBuffer::reflectionFBO.colorTexture);
}

void Render::renderSceneModels(Scene &scene)
{
    for (auto model : scene.structModels)
    {
        Shader shader = Shader::load(model.shader);

        // Send light and view position to relevant shader
        shader.setVec3("lightPos", EventHandler::lightPos);
        shader.setVec3("viewPos", EventHandler::cameraPosition);
        shader.setFloat("lightIntensity", 2.0f);
        shader.setVec3("lightCol", 1.f, 1.f, 1.f);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", model.u_model);
        shader.setMat4("u_normal", model.u_normal);

        renderModel(model.model);
    }
}

void Render::renderSceneUnitPlanes(Scene &scene)
{
    // Render opaque planes
    for (auto unitPlane : scene.opaqueUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        renderMesh(unitPlane.unitPlane);
    }

    // Sort transparent planes back to front based on distance from the camera
    std::sort(scene.transparentUnitPlanes.begin(), scene.transparentUnitPlanes.end(), [&](const UnitPlaneData &a, const UnitPlaneData &b)
              {
                  float distA = glm::distance(EventHandler::cameraPosition, a.position);
                  float distB = glm::distance(EventHandler::cameraPosition, b.position);
                  return distA > distB; // Sort by distance: farthest first, closest last
              });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render transparent planes
    for (auto unitPlane : scene.transparentUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        if (unitPlane.shader == "water" & FrameBuffer::Water == false)
        {
            FrameBuffer::WaterFrameBuffers();
        }
        renderMesh(unitPlane.unitPlane);
    }
    glDisable(GL_BLEND);
}

void Render::renderModel(Model *model)
{
    for (unsigned int i = 0; i < model->meshes.size(); i++)

        Render::renderMesh(model->meshes[i]);
}

// Render mesh
void Render::renderMesh(Mesh mesh)
{
    if (mesh.shader == "pbr")
    {
        renderPBR(mesh);
    }
    if (mesh.shader == "default")
    {
        renderDefault(mesh);
    }

    else if (mesh.shader == "simple")
    {
        renderSimple(mesh);
    }
}
void Render::renderDefault(Mesh mesh)
{
    Shader shader = Shader::load("default");
    unsigned int diffuseNr = 1;

    // For every texture
    for (unsigned int i = 0; i < mesh.textures.size(); i++)
    {
        // Activate texture unit before binding
        glActiveTexture(GL_TEXTURE0 + i);

        // Retrieve texture number and type
        std::string number;
        std::string name = mesh.textures[i].type;

        // Set appropriate number for filename (eg texture_diffuse3)
        if (name == "diffuse")
        {
            number = std::to_string(diffuseNr++);
            shader.setInt(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
        }
    }
    // Unload texture
    glActiveTexture(GL_TEXTURE0);

    // Draw Mesh
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Render::renderPBR(Mesh mesh)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int aoNr = 1;

    // For every texture
    for (unsigned int i = 0; i < mesh.textures.size(); i++)
    {
        // Activate texture unit before binding
        glActiveTexture(GL_TEXTURE0 + i);

        // Retrieve texture number and type
        std::string number;
        std::string name = mesh.textures[i].type;

        // Set appropriate number for filename (eg texture_diffuse3)
        if (name == "diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "specular")
            number = std::to_string(specularNr++);
        else if (name == "normal")
            number = std::to_string(normalNr++);
        else if (name == "roughness")
            number = std::to_string(roughnessNr++);
        else if (name == "ao")
            number = std::to_string(aoNr++);

        // Send texture to shader
        Shader::load("pbr").setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }
    // Unload texture
    glActiveTexture(GL_TEXTURE0);

    // Draw Mesh
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Render::renderSimple(Mesh mesh)
{
    // Draw Mesh
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Render::renderReflectRefract(Scene &scene)
{
    // Bind reflection buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::reflectionFBO);

    // Draw to it
    renderSceneModels(scene);
    renderSceneUnitPlanes(scene);

    // Bind refraction buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::refractionFBO);

    // Draw to it
    renderSceneModels(scene);
    renderSceneUnitPlanes(scene);

    // Unbind buffers, bind default one
    FrameBuffer::unbindCurrentFrameBuffer();
}

void Render::renderTestQuad(GLuint texture)
{
    glViewport(0, 0, EventHandler::screenWidth / 3, EventHandler::screenHeight / 3);

    Shader quadShader = Shader::load("gui");
    quadShader.setInt("screenTexture", 0);

    // Bind the framebuffer texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // restore viewport
    glViewport(0, 0, EventHandler::screenWidth, EventHandler::screenHeight);
}