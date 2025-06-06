#include "render/render.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <future>
#include <iostream>
#include <algorithm>

#include "camera/camera.hpp"
#include "debug/debug.hpp"
#include "event_handler/event_handler.hpp"
#include "framebuffer/framebuffer_util.hpp"
#include "model/model.hpp"
#include "model/bone.h"
#include "scene/scene.hpp"
#include "scene_manager/scene_manager.hpp"
#include "scene_manager/scene_manager_defs.h"
#include "shader/shader.hpp"
#include "shader/shader_util.hpp"
#include "texture_manager/texture_manager.hpp"
#include "thread_manager/thread_manager.hpp"
#include "ui_manager/ui_manager.hpp"
#include "easing_functions.h"

// Local variables
unsigned int quadVAO = 0, quadVBO = 0;
float quadVertices[] = {0};
bool WaterPass = true;
glm::vec3 debugColor(1.0f, 0.1f, 0.1f);
float textTextureSize = 128;

// Track current and last used shader
Shader *shader;
Shader *lastShader = nullptr;

void renderModel(const RenderCommand &cmd)
{
    // Send general shader data
    if (shader != lastShader)
    {
        // Set texture array index
        shader->setInt("textureArray", cmd.textureUnit);

        // Send light and view position to shader
        shader->setVec3("lightPos", EventHandler::lightPos);
        shader->setVec3("viewPos", Camera::getPosition());
        shader->setFloat("lightIntensity", EventHandler::lightInsensity);
        shader->setVec3("lightCol", EventHandler::lightCol);

        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Set clipping plane
        shader->setVec4("location_plane", Render::clipPlane);

        lastShader = shader;
    }

    // Send model specific data
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    shader->setIntArray("textureLayers", cmd.textureLayers.data(), cmd.textureLayers.size());

    shader->setBool("animated", cmd.animated);

    if (cmd.animated)
    {
        shader->setMat4Array("u_boneTransforms", cmd.boneTransforms);
        shader->setMat4Array("u_inverseOffsets", cmd.boneInverseOffsets);
    }

    // Draw meshes
    for (auto &mesh : *cmd.meshes)
    {
        std::visit([](auto &actualMesh)
                   { actualMesh.draw(); },
                   mesh);
    }
}

void renderOpaquePlane(const RenderCommand &cmd)
{
    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", Render::clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    if (cmd.shader == shaderID::shToonWater)
    {
        int toonWater = TextureManager::getStandaloneTextureUnit("../resources/textures/toonWater.jpeg");
        int normalMap = TextureManager::getStandaloneTextureUnit("../resources/textures/waterNormal.png");
        int heightmap = TextureManager::getStandaloneTextureUnit("../resources/textures/heightmap.jpg");

        shader->setInt("toonWater", toonWater);
        shader->setInt("normalMap", normalMap);
        shader->setInt("heightmap", heightmap);

        shader->setFloat("moveOffset", EventHandler::time);
        shader->setMat4("u_camXY", Camera::u_camXY);
    }

    // Draw meshes
    for (auto &mesh : *cmd.meshes)
    {
        std::visit([](auto &actualMesh)
                   { actualMesh.draw(); },
                   mesh);
    }
}

void renderTransparentPlane(const RenderCommand &cmd)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", Render::clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    if (cmd.shader == shaderID::shWater)
    {
        // Load surface textures
        unsigned int waterTexArrayID = TextureManager::getTextureArrayUnit("waterTextureArray");
        int dudv = TextureManager::getTextureLayerIndex("waterTextureArray", "../resources/textures/waterDUDV.png");
        int normal = TextureManager::getTextureLayerIndex("waterTextureArray", "../resources/textures/waterNormal.png");

        shader->setInt("waterTextureArray", waterTexArrayID);
        shader->setInt("dudvMapLayer", dudv);
        shader->setInt("normalMapLayer", normal);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, FramebufferUtil::reflectionFBO.colorTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, FramebufferUtil::refractionFBO.colorTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, FramebufferUtil::refractionFBO.depthTexture);

        shader->setInt("reflectionTexture", 1);
        shader->setInt("refractionTexture", 2);
        shader->setInt("depthMap", 3);
        shader->setFloat("moveOffset", EventHandler::time);
        shader->setVec3("cameraPosition", Camera::getPosition());
        shader->setVec3("lightPos", EventHandler::lightPos);
        shader->setVec3("lightCol", EventHandler::lightCol);
        shader->setMat4("u_camXY", Camera::u_camXY);
    }

    // Draw meshes
    if (cmd.shader == shaderID::shWater && !WaterPass)
    {
        for (auto &mesh : *cmd.meshes)
        {
            std::visit([](auto &actualMesh)
                       { actualMesh.draw(); },
                       mesh);
        }
    }

    glDisable(GL_BLEND);
}

void renderGrid(const RenderCommand &cmd)
{
    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", Render::clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    shader->setFloat("lod", cmd.lod);

    if (cmd.shader == shaderID::shToonTerrain)
    {
        int heightmap = TextureManager::getStandaloneTextureUnit("../resources/textures/heightmap.jpg");
        shader->setInt("heightmap", heightmap);

        shader->setMat4("u_camXY", Camera::u_camXY);
    }

    // Draw meshes
    for (auto &mesh : *cmd.meshes)
    {
        std::visit([](auto &actualMesh)
                   { actualMesh.draw(); },
                   mesh);
    }
}

void renderObjects(std::vector<RenderCommand> &renderBuffer)
{
    glPolygonMode(GL_FRONT_AND_BACK, Debug::wireMode ? GL_LINE : GL_FILL);
    glDisable(GL_CULL_FACE);

    for (const RenderCommand &cmd : renderBuffer)
    {
        shader = ShaderUtil::load(cmd.shader);

        switch (cmd.type)
        {
        case RenderType::rtModel:
            renderModel(cmd);
            break;

        case RenderType::rtOpaquePlane:
            renderOpaquePlane(cmd);
            break;

        case RenderType::rtTransparentPlane:
            renderTransparentPlane(cmd);
            break;

        case RenderType::rtGrid:
            renderGrid(cmd);
            break;
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
}

void renderSceneSkyBox()
{
    if (SceneManager::currentScene.get()->hasSkyBox)
    {
        // Disable depth test
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_DEPTH_TEST);

        // Load shader
        shader = ShaderUtil::load(shaderID::shSkybox);

        // Bind skybox
        glBindVertexArray(SceneManager::currentScene.get()->skyBox.VAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, SceneManager::currentScene.get()->skyBox.textureID);

        // Set view matrices
        shader->setMat4("u_view", glm::mat4(glm::mat3(Camera::u_view)));
        shader->setMat4("u_projection", Camera::u_projection);
        shader->setMat4("u_model", glm::mat4(1.0f));

        shader->setInt("skybox", 1);

        lastShader = shader;

        // Draw
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Enable depth test
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(0);
    }
}

void renderSceneTexts()
{
    for (auto text : SceneManager::currentScene.get()->texts)
    {
        Render::renderText(text.text, text.position.x, text.position.y, text.scale, text.color);
    }
}

void renderImage(const std::string &fileName, const glm::vec2 &position, const float width, const float height, const float alpha = 1.0f, const glm::vec2 scale = {1.0, 1.0f}, const float rotation = 0.0f, const bool mirrored = false)
{
    shader = ShaderUtil::load(shaderID::shImage);
    lastShader = shader;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(quadVAO);

    shader->setVec2("uScreenSize", glm::vec2(EventHandler::screenWidth, EventHandler::screenHeight));
    glm::vec2 screenSize = glm::vec2(EventHandler::screenWidth, EventHandler::screenHeight);

    unsigned int textureUnit = TextureManager::getStandaloneTextureUnit("../resources/images/" + fileName);
    shader->setInt("uTexture", textureUnit);

    glm::vec2 posFactor = position; // normalized 0..1

    float scaleX = EventHandler::screenWidth / 2560.0f;
    float scaleY = EventHandler::screenHeight / 1440.0f;
    glm::vec2 scaleFactors(scaleX, scaleY);

    glm::vec2 scaledScreenSize = glm::vec2(2560.0f, 1440.0f) * scaleFactors;
    glm::vec2 imageSizePx = glm::vec2(width, height) * scale * scaleFactors;

    glm::vec2 positionPx;
    positionPx.x = posFactor.x * (scaledScreenSize.x - imageSizePx.x);
    positionPx.y = posFactor.y * (scaledScreenSize.y - imageSizePx.y);

    shader->setVec2("uPosition", positionPx);

    shader->setVec2("uImageSize", glm::vec2(width, height));
    shader->setVec2("uScale", scale);
    shader->setFloat("uAlpha", alpha);
    shader->setFloat("uRotation", glm::radians(rotation));
    shader->setBool("uMirrored", mirrored);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void renderSceneImages()
{
    for (ImageData image : SceneManager::currentScene.get()->images)
    {
        renderImage(image.file, image.position, image.width, image.height, image.alpha, image.scale, image.rotation, image.mirrored);
    }
}

void renderReflectRefract(std::vector<RenderCommand> &renderBuffer)
{
    // ===== REFLECTOIN =====
    // Bind reflection buffer
    FramebufferUtil::bindFrameBuffer(FramebufferUtil::reflectionFBO);

    Render::clipPlane = {0, 0, 1, -Render::waterHeight};
    Camera::setCamDirection(glm::vec3(-Camera::getRotation()[0], Camera::getRotation()[1], Camera::getRotation()[2]));
    float distance = 2 * (Camera::getPosition()[2] - Render::waterHeight);
    Camera::genViewMatrix(Camera::getPosition() + glm::vec3(0, 0, -distance));

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox();
    glEnable(GL_CLIP_DISTANCE0);
    renderObjects(renderBuffer);
    glDisable(GL_CLIP_DISTANCE0);

    // ===== REFRACTION =====
    // Bind refraction buffer
    FramebufferUtil::bindFrameBuffer(FramebufferUtil::refractionFBO);

    Render::clipPlane = {0, 0, -1, Render::waterHeight};
    Camera::setCamDirection(Camera::getRotation());
    Camera::genViewMatrix(Camera::getPosition());

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox();
    glEnable(GL_CLIP_DISTANCE0);
    renderObjects(renderBuffer);
    glDisable(GL_CLIP_DISTANCE0);

    // Unbind buffers, bind default one
    FramebufferUtil::unbindCurrentFrameBuffer();
}

void renderTestQuad(unsigned int texture, int x, int y)
{
    glViewport(x, y, EventHandler::screenWidth / 3, EventHandler::screenHeight / 3);

    // Bind the framebuffer texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

    Shader *quadShader = ShaderUtil::load(shaderID::shGui);
    quadShader->setInt("screenTexture", 1);

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // restore viewport
    glViewport(0, 0, EventHandler::screenWidth, EventHandler::screenHeight);
}

void Render::setup()
{
    initQuad();
    initFreeType();
    createSceneFBO(EventHandler::windowWidth, EventHandler::windowHeight);
}

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

void Render::render()
{
    int currentIndex = Render::renderIndex.load(std::memory_order_acquire);
    auto &buffer = Render::renderBuffers[currentIndex];

    // Only render if the buffer is ready
    BufferState expected = BufferState::Ready;
    if (buffer.state.compare_exchange_strong(expected, BufferState::Rendering))
    {
        // Perform actual rendering
        Render::executeRender(buffer);

        // After rendering is done, mark buffer free
        buffer.state.store(BufferState::Free, std::memory_order_release);

        // Rotate render index to the next ready buffer (if available)
        for (int i = 0; i < 3; ++i)
        {
            if (Render::renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Ready)
            {
                Render::renderIndex.store(i, std::memory_order_release);
                break;
            }
        }

        ThreadManager::renderBufferCV.notify_all();
    }
}

void Render::prepareRender(RenderBuffer &prepBuffer)
{
    // Clear and reserve size for buffer
    prepBuffer.commandBuffer.clear();
    prepBuffer.commandBuffer.reserve(
        SceneManager::currentScene->structModels.size() +
        SceneManager::currentScene->opaqueUnitPlanes.size() +
        SceneManager::currentScene->transparentUnitPlanes.size() +
        SceneManager::currentScene->grids.size());

    std::vector<std::future<RenderCommand>> futures;

    // Load Models
    for (auto &model : SceneManager::currentScene->structModels)
    {
        futures.push_back(std::async(std::launch::async, [&model]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::rtModel;

            cmd.shader = model.shader; 
            
            cmd.modelMatrix = model.u_model;
            cmd.normalMatrix = model.u_normal;

            TextureManager::getTextureData(*model.model, cmd.textureUnit, cmd.textureArrayID, cmd.textureLayers);

            cmd.animated = model.animated;

            if (cmd.animated)
            {
                cmd.boneTransforms = model.model->getReadBuffer();
                cmd.boneInverseOffsets = model.model->boneInverseOffsets;
            }

            float distanceFromCamera = glm::distance(glm::vec3(model.u_model[3]), Camera::getPosition());

            cmd.lod = 0;
            if (distanceFromCamera > lodDistance) cmd.lod = 1;
            if (SceneManager::engineState == EngineState::esTitle) cmd.lod = 0;

            if (cmd.lod >= model.model->lodMeshes.size())
                cmd.lod = static_cast<int>(std::round(model.model->lodMeshes.size())) - 1;

            cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->lodMeshes[cmd.lod], [](std::vector<MeshVariant>*) {});

            return cmd; }));

        if (model.controlled)
        {
            prepBuffer.camPos = (model.u_model * model.model->getReadBuffer()[model.model->boneHierarchy["Armature_Cam"]->index]) * glm::vec4(0, 0, 0, 1);
            prepBuffer.camYaw = -atan2(model.u_model[0][1], model.u_model[1][1]);
        }
    }

    // Load opaque UnitPlanes
    for (auto &opaquePlane : SceneManager::currentScene->opaqueUnitPlanes)
    {
        futures.push_back(std::async(std::launch::async, [opaquePlane]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::rtOpaquePlane;

            cmd.shader = opaquePlane.shader;

            cmd.modelMatrix = opaquePlane.u_model;
            cmd.normalMatrix = opaquePlane.u_normal;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{opaquePlane.unitPlane});
            cmd.meshes = meshList;

            return cmd; }));
    }

    // Sort transparent planes back to front based on distance from the camera
    std::sort(SceneManager::currentScene->transparentUnitPlanes.begin(), SceneManager::currentScene->transparentUnitPlanes.end(), [&](const UnitPlaneData &a, const UnitPlaneData &b)
              {
                  float distA = glm::distance(Camera::getPosition(), a.position);
                  float distB = glm::distance(Camera::getPosition(), b.position);
                  return distA > distB; // Sort by distance: farthest first, closest last
              });

    // Load transparent UnitPlanes
    for (auto &transparentPlane : SceneManager::currentScene->transparentUnitPlanes)
    {
        futures.push_back(std::async(std::launch::async, [transparentPlane]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::rtTransparentPlane;

            cmd.shader = transparentPlane.shader;

            cmd.modelMatrix = transparentPlane.u_model;
            cmd.normalMatrix = transparentPlane.u_normal;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{transparentPlane.unitPlane});
            cmd.meshes = meshList;

            return cmd; }));
    }

    // Load grids
    for (auto &grid : SceneManager::currentScene->grids)
    {
        futures.push_back(std::async(std::launch::async, [grid]()
                                     {
            RenderCommand cmd;
            
            cmd.type = RenderType::rtGrid;

            cmd.shader = grid.shader;

            cmd.modelMatrix = grid.u_model;
            cmd.normalMatrix = grid.u_normal;

            cmd.lod = grid.lod;

            auto meshList = std::make_shared<std::vector<MeshVariant>>(std::initializer_list<MeshVariant>{grid.grid});
            cmd.meshes = meshList;

            return cmd; }));
    }

    for (auto &f : futures)
    {
        prepBuffer.commandBuffer.push_back(f.get());
    }
}

void Render::executeRender(RenderBuffer &renderBuffer, bool toScreen)
{
    // Set camera from buffer
    Camera::cameraPosition = renderBuffer.camPos;
    Camera::yaw = renderBuffer.camYaw;
    Camera::update();

    // Bind to render buffer
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // Clear color buffer
    glClearColor(SceneManager::currentScene->bgColor.r, SceneManager::currentScene->bgColor.g, SceneManager::currentScene->bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderSceneSkyBox();

    // If water loaded, render buffers
    if (ShaderUtil::waterLoaded && (EventHandler::frame % 2 == 0 || !toScreen))
    {
        WaterPass = true;
        renderReflectRefract(renderBuffer.commandBuffer);
        WaterPass = false;
    }

    // Reset clip plane
    clipPlane = {0, 0, 0, 0};

    // Render rest of scene
    renderObjects(renderBuffer.commandBuffer);

    // Render debug menu
    if (SceneManager::engineState == EngineState::esRunning)
    {
        // Set debug data
        std::string debugText;
        FPS = (0.9f * FPS + 0.1f / EventHandler::deltaTime);

        // Select which debug renderer to use
        switch (debugstate)
        {
        case debugState::dbNone:
            break;

        case debugState::dbFPS:
            renderText(std::to_string(static_cast<int>(FPS)), 0.01f, 0.01f, 0.75f, debugColor);
            break;

        case debugState::dbPhysics:
            debugText = "Physics:\n";

            for (auto entry : debugPhysicsData)
            {
                debugText = debugText + entry.first + ": " + std::to_string(entry.second) + "\n";
            }

            renderText(debugText, 0.01f, 0.01f, 1, debugColor);
            break;
        }
    }

    renderSceneTexts();
    renderSceneImages();

    savePauseBackground();

    if (toScreen)
    {
        // Switch back to screenbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader = ShaderUtil::load(shaderID::shPost);
        shader->setInt("screenTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    Camera::cameraMoved = false;
    lastShader = nullptr;
}

void Render::initFreeType()
{
    // Initialize FreeType
    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR: Could not initialize FreeType\n";

    const std::string fontPath = "../" + fontpath;

    // Load font face
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        std::cerr << "ERROR: Failed to load font\n";

    // Set font size
    FT_Set_Pixel_Sizes(face, 0, textTextureSize);

    // Generate texture for each character
    glGenTextures(1, &textTexture);
    glBindTexture(GL_TEXTURE_2D, textTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Store the texture in your character map
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // To prevent padding issues

    // Define the width and height of the atlas
    const int atlasWidth = textTextureSize * 10;
    const int atlasHeight = textTextureSize * 10;
    GLubyte *atlasData = new GLubyte[atlasWidth * atlasHeight * 4]; // RGBA

    int xPos = 0, yPos = 0;
    const int gapX = 2; // Define a small gap between characters
    const int gapY = textTextureSize;

    for (unsigned int c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR: Failed to load Glyph\n";
            continue;
        }

        // Check if the character fits in the current line
        if (xPos + face->glyph->bitmap.width + gapX >= atlasWidth)
        {
            xPos = 0;
            yPos += gapY;
        }

        // Copy the character bitmap into the atlasData
        for (int y = 0; y < face->glyph->bitmap.rows; y++)
        {
            for (int x = 0; x < face->glyph->bitmap.width; x++)
            {
                int index = (yPos + y) * atlasWidth * 4 + (xPos + x) * 4;
                atlasData[index] = 255;                                                               // R
                atlasData[index + 1] = 255;                                                           // G
                atlasData[index + 2] = 255;                                                           // B
                atlasData[index + 3] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x]; // A
            }
        }

        // Store character information in the map
        Character ch = {
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
            glm::vec4(xPos / (float)atlasWidth, yPos / (float)atlasHeight, face->glyph->bitmap.width / (float)atlasWidth, face->glyph->bitmap.rows / (float)atlasHeight)};
        Characters[c] = ch;

        // Update xPos for the next character in the atlas
        xPos += face->glyph->bitmap.width + gapX; // Add gap to x position
    }

    // Now upload the entire atlas texture to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData);
    delete[] atlasData; // Free the memory
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate VAO and VBO for text rendering
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    // Set up vertex attributes (assuming position and texture coordinates)
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

void Render::renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha)
{
    x *= EventHandler::screenUIScale * 2560.0f;
    y *= EventHandler::screenUIScale * 1440.0f;
    scale *= EventHandler::screenUIScale;

    // Load the shader for rendering text
    shader = ShaderUtil::load(shaderID::shText);

    if (shader != lastShader)
    {
        // Set the projection matrix for the text shader
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(EventHandler::screenWidth), static_cast<float>(EventHandler::screenHeight), 0.0f);
        shader->setMat4("projection", projection);

        lastShader = shader;
    }

    // Set text color uniform
    shader->setVec3("textColor", color.r, color.g, color.b);
    shader->setFloat("textAlpha", alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Render::textTexture);

    glBindVertexArray(Render::textVAO);

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float startX = x;                                                  // Store the initial x position
    float lineSpacing = Render::Characters['H'].Size.y * scale * 1.5f; // Adjust line spacing with a small padding

    for (char c : text)
    {
        if (c == '\n')
        {
            x = startX;       // Reset x to the start of the line
            y += lineSpacing; // Move y down by the height of a character plus padding
            continue;
        }

        Character ch = Render::Characters[c];

        // Calculate the position of each character
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (Render::Characters['H'].Size.y - ch.Bearing.y) * scale; // Adjust y-coordinate calculation
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Prepare the vertices and texture coordinates with counter-clockwise winding
        float vertices[6][4] = {
            {xpos, ypos + h, ch.TexCoords.x, ch.TexCoords.y + ch.TexCoords.w}, // Bottom-left
            {xpos + w, ypos, ch.TexCoords.x + ch.TexCoords.z, ch.TexCoords.y}, // Top-right
            {xpos, ypos, ch.TexCoords.x, ch.TexCoords.y},                      // Top-left

            {xpos, ypos + h, ch.TexCoords.x, ch.TexCoords.y + ch.TexCoords.w},                      // Bottom-left
            {xpos + w, ypos + h, ch.TexCoords.x + ch.TexCoords.z, ch.TexCoords.y + ch.TexCoords.w}, // Bottom-right
            {xpos + w, ypos, ch.TexCoords.x + ch.TexCoords.z, ch.TexCoords.y}                       // Top-right
        };

        // Update VBO with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, Render::textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    // Unbind the texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Disable blending after text rendering
    glDisable(GL_BLEND);
}

void Render::createSceneFBO(int width, int height)
{
    glGenTextures(1, &pauseTexture);
    glBindTexture(GL_TEXTURE_2D, pauseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, EventHandler::screenWidth, EventHandler::screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &copyFBO);

    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    // Color texture
    glGenTextures(1, &sceneTexture);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Good for post
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);

    // Depth renderbuffer
    glGenRenderbuffers(1, &sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Scene FBO is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::resize(int width, int height)
{
    // Delete old FBO attachments
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneTexture);
    glDeleteRenderbuffers(1, &sceneDepthRBO);

    glDeleteFramebuffers(1, &copyFBO);
    glDeleteTextures(1, &pauseTexture);

    createSceneFBO(width, height);
}

void Render::savePauseBackground()
{
    // Bind FBOs
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copyFBO);

    // Attach target texture to copy FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pauseTexture, 0);

    // Check framebuffer completeness (optional but useful for debugging)
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Copy FBO is not complete!" << std::endl;

    // Perform blit
    glBlitFramebuffer(
        0, 0, EventHandler::screenWidth, EventHandler::screenHeight,
        0, 0, EventHandler::screenWidth, EventHandler::screenHeight,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::renderBlankScreen()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::renderLoadingScreen()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::string progressString;
    std::string statusString = "Loading...";

    float effectiveFade;

    std::vector<LoadingStep> loadingSteps = {
        {"Unloaded Success", []
         { return "Clearing Previous"; }},
        {"Scene JSON Complete", []
         { return "Loading new Scene JSON"; }},
        {"Background Colors Complete", []
         { return "Loading Background Colors"; }},
        {"Texts Complete", [&]
         { return "Loading Texts [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Images Complete", [&]
         { return "Loading Images [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Models Complete", [&]
         { return "Loading Models [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Planes Complete", [&]
         { return "Loading Planes [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Terrain Grids Complete", [&]
         { return "Loading Terrain Grids [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Skybox Complete", []
         { return "Loading Skybox"; }},
        {"Textures Complete", [&]
         { return "Loading Textures [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"OpenGL Upload Complete", []
         { return "Uploading to OpenGL"; }}};

    // Render completed steps
    for (int i = 0; i < SceneManager::loadingState - 1 && i < loadingSteps.size(); ++i)
    {
        progressString += loadingSteps[i].completedLabel + "\n";
    }

    // Render current step
    if (SceneManager::loadingState >= 1 && SceneManager::loadingState <= loadingSteps.size())
    {
        progressString += loadingSteps[SceneManager::loadingState - 1].activeMessage() + "\n";
    }

    // Loading complete
    if (SceneManager::loadingState == 100)
        statusString = "Finished Loading";

    renderText(statusString, 0.05f, 0.05f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(progressString, 0.05f, 0.20f, 0.5f, glm::vec3(0.6f, 0.1f, 0.1f));

    // Render first frame under it
    if (SceneManager::engineState == EngineState::esLoading)
        effectiveFade = 1.0f;
    else
        effectiveFade = std::clamp(SceneManager::menuFade, 0.0f, 1.0f);

    float darkfactor = easeInOutQuad(0.0f, 1.0f, effectiveFade);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pauseTexture);

    shader = ShaderUtil::load(shaderID::shDarkenBlur);
    shader->setInt("screenTexture", 0);
    shader->setVec2("texelSize", glm::vec2(1.0f / EventHandler::screenWidth, 1.0f / EventHandler::screenHeight));
    shader->setFloat("darkenAmount", darkfactor);
    shader->setFloat("darkenPosition", 2);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render::renderTitleScreen()
{
    float effectiveFade = std::clamp(SceneManager::menuFade, 0.0f, 1.0f);

    float color = easeInOutQuad(0.0f, 0.5f, effectiveFade);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(color, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    float imageAlpha = easeInOutQuad(0.0f, 1.0f, effectiveFade);

    float xpos;
    if (SceneManager::exitState == EngineState::esTitle)
        xpos = easeOutCubic(1.33f, 0.67f, effectiveFade);
    else
        xpos = easeOutCubic(0.0f, 0.67f, effectiveFade);

    renderImage("title-figure-black.png", glm::vec2(xpos, 0.5f) + glm::vec2(0.003f, -0.01f), 835, 1024, imageAlpha);
    renderImage("title-figure.png", glm::vec2(xpos, 0.5f), 835, 1024, imageAlpha);

    float fadeOffset = 0.3333f;
    std::vector<glm::vec2> offsets;
    std::vector<float> alphas;

    for (int i = 0; i < 1 + UIManager::buttons.size(); i++)
    {
        if (SceneManager::exitState == EngineState::esPause)
            effectiveFade = std::clamp(SceneManager::menuFade, 0.0f, 1.0f);
        else
            effectiveFade = std::clamp(SceneManager::menuFade - fadeOffset * i, 0.0f, 1.0f);

        alphas.push_back(easeOutCubic(0.0f, 1.0f, effectiveFade));

        float x = easeOutBack(-0.1f, 0.0f, effectiveFade, 2.0f);
        float y = 0.0f;

        offsets.push_back(glm::vec2(x, y));
    }

    renderText("Land Yachting Simulator", 0.033f + offsets[0].x, 0.053f, 1, glm::vec3(0.0f, 0.0f, 0.0f), alphas[0]);
    renderText("Land Yachting Simulator", 0.03f + offsets[0].x, 0.05f, 1, glm::vec3(1.0f, 1.0f, 1.0f), alphas[0]);

    for (int i = 0; i < UIManager::buttons.size(); i++)
    {
        UIManager::buttons[i].setOffset(offsets[i + 1]);
        UIManager::buttons[i].setAlpha(alphas[i + 1]);
    }

    glEnable(GL_DEPTH_TEST);
}

void Render::renderMenuScreen(const EngineState &state, const SettingsPage &page)
{
    float maxDarkFactor = 0.8f;

    float effectiveFade = std::clamp(SceneManager::menuFade, 0.0f, 1.0f);

    float darkfactor;

    if (SceneManager::engineState == EngineState::esTitleSettings)
        darkfactor = 1.0f;
    else
        darkfactor = easeInOutQuad(0.0f, maxDarkFactor, effectiveFade);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pauseTexture);

    shader = ShaderUtil::load(shaderID::shDarkenBlur);
    shader->setInt("screenTexture", 0);
    shader->setVec2("texelSize", glm::vec2(1.0f / EventHandler::screenWidth, 1.0f / EventHandler::screenHeight));
    shader->setFloat("darkenAmount", darkfactor);
    shader->setFloat("darkenPosition", 0.4);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    float fadeOffset = 0.1;
    std::vector<glm::vec2> offsets;
    std::vector<float> alphas;

    for (int i = 0; i < 1 + UIManager::uiElements.size(); i++)
    {
        if (SceneManager::exitState == EngineState::esPause)
            effectiveFade = std::clamp(SceneManager::menuFade, 0.0f, 1.0f);
        else
            effectiveFade = std::clamp(SceneManager::menuFade - fadeOffset * i, 0.0f, 1.0f);

        alphas.push_back(easeOutCubic(0.0f, 1.0f, effectiveFade));

        float x = easeOutBack(-0.1f, 0.0f, effectiveFade, 2.0f);
        float y = 0.0f;

        offsets.push_back(glm::vec2(x, y));
    }

    std::string header;

    switch (state)
    {
    case EngineState::esPause:
        header = "Paused";
        break;

    case EngineState::esSettings:
    case EngineState::esTitleSettings:
        switch (page)
        {
        case SettingsPage::spStart:
            header = "Settings";
            break;

        case SettingsPage::spGraphics:
            header = "Settings - Graphics";
            break;

        case SettingsPage::spPhysics:
            header = "Settings - Physics";
            break;

        case SettingsPage::spDebug:
            header = "Settings - Debug";
            break;
        }

        break;
    }

    renderText(header, 0.033f + offsets[0].x, 0.053f, 1, glm::vec3(0.0f, 0.0f, 0.0f), alphas[0]);
    renderText(header, 0.03f + offsets[0].x, 0.05f, 1, glm::vec3(1.0f, 1.0f, 1.0f), alphas[0]);

    for (int i = 0; i < UIManager::uiElements.size(); i++)
    {
        std::visit([&](auto *element)
                   { element->setOffset(offsets[i + 1]);
                    element->setAlpha(alphas[i+1]); },
                   UIManager::uiElements[i]);
    }

    glEnable(GL_DEPTH_TEST);
}