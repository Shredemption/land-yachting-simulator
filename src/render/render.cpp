#include "render/render.h"

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <format>

#include "event_handler/event_handler.h"
#include "frame_buffer/frame_buffer.h"
#include "camera/camera.h"
#include "scene_manager/scene_manager.h"
#include "texture_manager/texture_manager.h"

// Global variables for quads
unsigned int Render::quadVAO = 0, Render::quadVBO = 0;
float Render::quadVertices[] = {0};

// Water variables
float Render::waterHeight = 0.25;
bool Render::WaterPass = true;

// Render states
debugState Render::debugState;
float Render::FPS = 0.0f;
std::vector<std::pair<std::string, float>> Render::debugPhysicsData;
glm::vec3 debugColor(1.0f, 0.1f, 0.1f);
std::function<void(std::string)> updateTimingFunc = [](std::string) {};

glm::vec4 Render::clipPlane(0, 0, 0, 0);
float Render::lodDistance = 30.0f;

FT_Library Render::ft;
FT_Face Render::face;

// Text variables
GLuint Render::textVAO, Render::textVBO;
GLuint Render::textTexture;
std::map<GLchar, Character> Render::Characters;
std::string Render::fontpath = "resources/fonts/MusticaPro-SemiBold.otf";

// Timing variables
std::chrono::high_resolution_clock::time_point lastCPUTime;
GLuint lastGPUQuery = 0;

// Track current and last used shader
Shader *shader;
Shader *lastShader = nullptr;

std::vector<RenderCommand> Render::prepBuffer;
std::vector<RenderCommand> Render::renderBuffer;

// Setup quads and text
void Render::setup()
{
    Render::initQuad();
    Render::initFreeType();
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

void Render::prepareRender()
{
    // Clear and reserve size for buffer
    prepBuffer.clear();
    prepBuffer.reserve(
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
                cmd.boneTransforms = &model.model->boneTransforms;
                cmd.boneInverseOffsets = &model.model->boneInverseOffsets;
            }

            float distanceFromCamera = glm::distance(glm::vec3(model.u_model[3]), Camera::getPosition());

            cmd.lod = 0;
            if (distanceFromCamera > lodDistance) cmd.lod = 1;
            if (SceneManager::onTitleScreen) cmd.lod = 0;

            if (cmd.lod >= model.model->lodMeshes.size())
                cmd.lod = static_cast<int>(std::round(model.model->lodMeshes.size())) - 1;

            cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->lodMeshes[cmd.lod], [](std::vector<MeshVariant>*) {});

            return cmd; }));
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
        prepBuffer.push_back(f.get());
    }
}

void Render::executeRender()
{
    // Clear color buffer
    glClearColor(SceneManager::currentScene->bgColor.r, SceneManager::currentScene->bgColor.g, SceneManager::currentScene->bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderSceneSkyBox();

    // If water loaded, render buffers
    if (Shader::waterLoaded && EventHandler::frame % 3 == 0)
    {
        WaterPass = true;
        renderReflectRefract();
        WaterPass = false;
    }

    // Reset clip plane
    clipPlane = {0, 0, 0, 0};

    // Render rest of scene
    renderObjects();

    // Render debug menu
    if (!SceneManager::onTitleScreen)
    {
        // Set debug data
        std::string debugText;
        FPS = (0.9f * FPS + 0.1f / EventHandler::deltaTime);

        // Select which debug renderer to use
        switch (debugState)
        {
        case dbNone:
            break;

        case dbFPS:
            renderText(std::to_string(static_cast<int>(FPS)), 0.01f, 0.01f, 0.75f, debugColor);
            break;

        case dbPhysics:
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

    Camera::cameraMoved = false;
    lastShader = nullptr;
}

void Render::renderObjects()
{
    for (const RenderCommand &cmd : renderBuffer)
    {
        shader = Shader::load(cmd.shader);

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
}

void Render::renderModel(const RenderCommand &cmd)
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
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Send model specific data
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    shader->setIntArray("textureLayers", cmd.textureLayers.data(), cmd.textureLayers.size());

    shader->setBool("animated", cmd.animated);

    if (cmd.animated)
    {
        shader->setMat4Array("u_boneTransforms", *cmd.boneTransforms);
        shader->setMat4Array("u_inverseOffsets", *cmd.boneInverseOffsets);
    }

    // Draw meshes
    for (auto &mesh : *cmd.meshes)
    {
        std::visit([](auto &actualMesh)
                   { actualMesh.draw(); },
                   mesh);
    }
}

void Render::renderOpaquePlane(const RenderCommand &cmd)
{
    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", clipPlane);

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

void Render::renderTransparentPlane(const RenderCommand &cmd)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    if (cmd.shader == shaderID::shWater)
    {
        // Load surface textures
        GLuint waterTexArrayID = TextureManager::getTextureArrayUnit("waterTextureArray");
        int dudv = TextureManager::getTextureLayerIndex("waterTextureArray", "../resources/textures/waterDUDV.png");
        int normal = TextureManager::getTextureLayerIndex("waterTextureArray", "../resources/textures/waterNormal.png");

        shader->setInt("waterTextureArray", waterTexArrayID);
        shader->setInt("dudvMapLayer", dudv);
        shader->setInt("normalMapLayer", normal);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, FrameBuffer::reflectionFBO.colorTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, FrameBuffer::refractionFBO.colorTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, FrameBuffer::refractionFBO.depthTexture);

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

void Render::renderGrid(const RenderCommand &cmd)
{
    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Clipping Plane
        shader->setVec4("location_plane", clipPlane);

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

void Render::renderSceneSkyBox()
{
    if (SceneManager::currentScene.get()->hasSkyBox)
    {
        // Disable depth test
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_DEPTH_TEST);

        // Load shader
        shader = Shader::load(shaderID::shSkybox);

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

void Render::renderSceneTexts()
{
    for (auto text : SceneManager::currentScene.get()->texts)
    {
        renderText(text.text, text.position.x, text.position.y, text.scale, text.color);
    }
}

void Render::renderSceneImages()
{
    shader = Shader::load(shaderID::shImage);
    lastShader = shader;

    shader->setVec2("uScreenSize", glm::vec2(EventHandler::screenWidth, EventHandler::screenHeight));
    glm::vec2 screenSize = glm::vec2(EventHandler::screenWidth, EventHandler::screenHeight);

    glBindVertexArray(quadVAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (ImageData image : SceneManager::currentScene.get()->images)
    {
        GLuint textureUnit = TextureManager::getStandaloneTextureUnit("../resources/images/" + image.file);
        shader->setInt("uTexture", textureUnit);

        glm::vec2 posFactor = image.position; // normalized 0..1

        glm::vec2 imageSizePx = glm::vec2(image.width, image.height) * image.scale;

        // Calculate center position so edges match posFactor
        glm::vec2 positionPx;
        positionPx.x = posFactor.x * (screenSize.x - imageSizePx.x);
        positionPx.y = posFactor.y * (screenSize.y - imageSizePx.y);

        shader->setVec2("uPosition", positionPx);

        shader->setVec2("uImageSize", glm::vec2(image.width, image.height));
        shader->setVec2("uScale", image.scale);
        shader->setFloat("uRotation", glm::radians(image.rotation));
        shader->setBool("uMirrored", image.mirrored);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void Render::renderReflectRefract()
{
    // ===== REFLECTOIN =====
    // Bind reflection buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::reflectionFBO);

    clipPlane = {0, 0, 1, -waterHeight};
    Camera::setCamDirection(glm::vec3(-Camera::getRotation()[0], Camera::getRotation()[1], Camera::getRotation()[2]));
    float distance = 2 * (Camera::getPosition()[2] - waterHeight);
    Camera::genViewMatrix(Camera::getPosition() + glm::vec3(0, 0, -distance));

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox();
    glEnable(GL_CLIP_DISTANCE0);
    renderObjects();
    glDisable(GL_CLIP_DISTANCE0);

    // ===== REFRACTION =====
    // Bind refraction buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::refractionFBO);

    clipPlane = {0, 0, -1, waterHeight};
    Camera::setCamDirection(Camera::getRotation());
    Camera::genViewMatrix(Camera::getPosition());

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox();
    glEnable(GL_CLIP_DISTANCE0);
    renderObjects();
    glDisable(GL_CLIP_DISTANCE0);

    // Unbind buffers, bind default one
    FrameBuffer::unbindCurrentFrameBuffer();
}

void Render::renderTestQuad(GLuint texture, int x, int y)
{
    glViewport(x, y, EventHandler::screenWidth / 3, EventHandler::screenHeight / 3);

    // Bind the framebuffer texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

    Shader *quadShader = Shader::load(shaderID::shGui);
    quadShader->setInt("screenTexture", 1);

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // restore viewport
    glViewport(0, 0, EventHandler::screenWidth, EventHandler::screenHeight);
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
    FT_Set_Pixel_Sizes(face, 0, 48);

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
    const int atlasWidth = 1024;
    const int atlasHeight = 1024;
    GLubyte *atlasData = new GLubyte[atlasWidth * atlasHeight * 4]; // RGBA

    int xPos = 0, yPos = 0;
    const int gapX = 2; // Define a small gap between characters
    const int gapY = 32;

    for (GLuint c = 0; c < 128; c++)
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
            yPos += face->glyph->bitmap.rows + gapY;
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
            static_cast<GLuint>(face->glyph->advance.x),
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

void Render::renderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    x *= EventHandler::screenHeight;
    y *= EventHandler::screenHeight;
    scale *= EventHandler::screenHeight / 1440.0f;

    // Load the shader for rendering text
    Shader *shader = Shader::load(shaderID::shText);

    if (shader != lastShader)
    {
        // Set the projection matrix for the text shader
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(EventHandler::screenWidth), static_cast<float>(EventHandler::screenHeight), 0.0f);
        shader->setMat4("projection", projection);

        lastShader = shader;
    }

    // Set text color uniform
    shader->setVec3("textColor", color.r, color.g, color.b);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    glBindVertexArray(textVAO);

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float startX = x;                                          // Store the initial x position
    float lineSpacing = Characters['H'].Size.y * scale * 1.5f; // Adjust line spacing with a small padding

    for (char c : text)
    {
        if (c == '\n')
        {
            x = startX;       // Reset x to the start of the line
            y += lineSpacing; // Move y down by the height of a character plus padding
            continue;
        }

        Character ch = Characters[c];

        // Calculate the position of each character
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (Characters['H'].Size.y - ch.Bearing.y) * scale; // Adjust y-coordinate calculation
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
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
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
