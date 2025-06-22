#include "render/render.hpp"

#include "pch.h"

#include "ui_manager/ui_manager_defs.h"

// Text
unsigned int textVAO, textVBO;
unsigned int textTexture;
std::map<GLchar, Character> Characters;
std::string fontpath = "resources/fonts/MusticaPro-SemiBold.otf";
float textTextureSize = 128;

FT_Library ft;
FT_Face face;

// Framebuffer
unsigned int sceneTexture = 0, sceneDepthRBO = 0;
unsigned int pauseTexture;
unsigned int copyFBO;

// Clipping and culling
glm::vec4 clipPlane(0, 0, 0, 0);

// Quad for rendering
unsigned int quadVAO = 0, quadVBO = 0;
float quadVertices[] = {0};

// Water
bool WaterPass = false;
float waterHeight = 0.25;
float waterTimer = 0.0f;

// Debug
glm::vec3 debugColor(1.0f, 0.1f, 0.1f);
float FPS = 0.0f;

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
        shader->setVec3("lightPos", SceneManager::currentScene.get()->lightPos);
        shader->setVec3("viewPos", Camera::getPosition());
        shader->setFloat("lightIntensity", SceneManager::currentScene.get()->lightInsensity);
        shader->setVec3("lightCol", SceneManager::currentScene.get()->lightCol);

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

    shader->setVec3("bodyColor", cmd.color);

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

void renderHitbox(const RenderCommand &cmd)
{
    if (WaterPass)
        return;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);

    // Send general shader data
    if (shader != lastShader)
    {
        // Apply view and projection to whole scene
        shader->setMat4("u_view", Camera::u_view);
        shader->setMat4("u_projection", Camera::u_projection);

        // Set clipping plane
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Send model specific data
    shader->setMat4("u_model", cmd.modelMatrix);

    shader->setBool("animated", cmd.animated);

    shader->setVec3("bodyColor", cmd.color);

    // Draw meshes
    for (auto &mesh : *cmd.meshes)
    {
        std::visit([](auto &actualMesh)
                   { actualMesh.draw(); },
                   mesh);
    }

    if (!SettingsManager::settings.debug.wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
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
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    if (cmd.shader == shaderID::ToonWater)
    {
        int toonWater = TextureManager::getStandaloneTextureUnit("resources/textures/toonWater.jpeg");
        int normalMap = TextureManager::getStandaloneTextureUnit("resources/textures/waterNormal.png");
        int heightmap = TextureManager::getStandaloneTextureUnit("resources/textures/heightmap.jpg");

        shader->setInt("toonWater", toonWater);
        shader->setInt("normalMap", normalMap);
        shader->setInt("heightmap", heightmap);

        shader->setFloat("moveOffset", TimeManager::time);
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
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    if (cmd.shader == shaderID::Water)
    {
        // Load surface textures
        unsigned int waterTexArrayID = TextureManager::getTextureArrayUnit("waterTextureArray");
        int dudv = TextureManager::getTextureLayerIndex("waterTextureArray", "resources/textures/waterDUDV.png");
        int normal = TextureManager::getTextureLayerIndex("waterTextureArray", "resources/textures/waterNormal.png");

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
        shader->setFloat("moveOffset", TimeManager::time);
        shader->setVec3("cameraPosition", Camera::getPosition());
        shader->setVec3("lightPos", SceneManager::currentScene.get()->lightPos);
        shader->setVec3("lightCol", SceneManager::currentScene.get()->lightCol);
        shader->setMat4("u_camXY", Camera::u_camXY);
    }

    // Draw meshes
    if (cmd.shader == shaderID::Water && !WaterPass)
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
        shader->setVec4("location_plane", clipPlane);

        lastShader = shader;
    }

    // Set model matrix for model and draw
    shader->setMat4("u_model", cmd.modelMatrix);
    shader->setMat4("u_normal", cmd.normalMatrix);

    shader->setFloat("lod", cmd.lod);

    if (cmd.shader == shaderID::ToonTerrain)
    {
        int heightmap = TextureManager::getStandaloneTextureUnit("resources/textures/heightmap.jpg");
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

    if (SettingsManager::settings.debug.wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    for (const RenderCommand &cmd : renderBuffer)
    {
        shader = ShaderUtil::load(cmd.shader);

        switch (cmd.type)
        {
        case RenderType::Model:
            renderModel(cmd);
            break;

        case RenderType::Hitbox:
            renderHitbox(cmd);
            break;

        case RenderType::OpaquePlane:
            renderOpaquePlane(cmd);
            break;

        case RenderType::TransparentPlane:
            renderTransparentPlane(cmd);
            break;

        case RenderType::Grid:
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
        shader = ShaderUtil::load(shaderID::Skybox);

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
    shader = ShaderUtil::load(shaderID::Image);
    lastShader = shader;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(quadVAO);

    shader->setVec2("uScreenSize", glm::vec2(WindowManager::screenWidth, WindowManager::screenHeight));
    glm::vec2 screenSize = glm::vec2(WindowManager::screenWidth, WindowManager::screenHeight);

    unsigned int textureUnit = TextureManager::getStandaloneTextureUnit("resources/images/" + fileName);
    shader->setInt("uTexture", textureUnit);

    glm::vec2 posFactor = position; // normalized 0..1

    float scaleX = WindowManager::screenWidth / 2560.0f;
    float scaleY = WindowManager::screenHeight / 1440.0f;
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

    clipPlane = {0, 0, 1, -waterHeight};
    Camera::setCamDirection(glm::vec3(-Camera::getRotation()[0], Camera::getRotation()[1], Camera::getRotation()[2]));
    float distance = 2 * (Camera::getPosition()[2] - waterHeight);
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

    clipPlane = {0, 0, -1, waterHeight};
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
    glViewport(x, y, WindowManager::screenWidth / 3, WindowManager::screenHeight / 3);

    // Bind the framebuffer texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

    Shader *quadShader = ShaderUtil::load(shaderID::Gui);
    quadShader->setInt("screenTexture", 1);

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // restore viewport
    glViewport(0, 0, WindowManager::screenWidth, WindowManager::screenHeight);
}

void initQuad()
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

void initFreeType()
{
    // Initialize FreeType
    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR: Could not initialize FreeType\n";

    const std::string fontPath = fontpath;

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

void createSceneFBO(int width, int height)
{
    // Pause Buffer
    glGenTextures(1, &pauseTexture);
    glBindTexture(GL_TEXTURE_2D, pauseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WindowManager::screenWidth, WindowManager::screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &copyFBO);

    // Scene Buffer
    glGenFramebuffers(1, &Render::sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Render::sceneFBO);

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

float calculateTextWidth(const std::string &text, float scale)
{
    float width = 0.0f;
    float lineWidth = 0.0f;

    for (char c : text)
    {
        if (c == '\n')
        {
            width = std::max(width, lineWidth);
            lineWidth = 0.0f;
            continue;
        }

        if (Characters.count(c) == 0)
            continue;
        Character ch = Characters[c];
        lineWidth += (ch.Advance >> 6) * scale;
    }

    width = std::max(width, lineWidth);
    return width;
}

void Render::setup()
{
    initQuad();
    initFreeType();
    createSceneFBO(WindowManager::windowWidth, WindowManager::windowHeight);

    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Enable Depth buffer (Z-buffer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
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

void Render::render()
{
    int currentIndex = renderIndex.load(std::memory_order_acquire);
    auto &buffer = renderBuffers[currentIndex];

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
            if (renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Ready)
            {
                renderIndex.store(i, std::memory_order_release);
                break;
            }
        }

        ThreadManager::renderBufferCV.notify_all();
    }
}

void Render::prepareRender(::RenderBuffer &prepBuffer)
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
            
            cmd.type = RenderType::Model;

            cmd.shader = model.shader; 
            cmd.color = model.color;
            
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
            if (distanceFromCamera > SettingsManager::settings.video.lodDistance) cmd.lod = 1;
            if (SceneManager::engineState == EngineState::Title) cmd.lod = 0;

            if (cmd.lod >= model.model->lodMeshes.size())
                cmd.lod = static_cast<int>(std::round(model.model->lodMeshes.size())) - 1;

            cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->lodMeshes[cmd.lod], [](std::vector<MeshVariant>*) {});

            return cmd; }));

        // Hitboxes
        if (SettingsManager::settings.debug.showHitboxes)
        {
            if (model.model->hitboxMeshes.has_value() && !model.model->hitboxMeshes->empty())
            {
                futures.push_back(std::async(std::launch::async, [&model]()
                                             {
                    RenderCommand cmd;
                    
                    cmd.type = RenderType::Hitbox;

                    cmd.shader = shaderID::Hitbox; 
                    cmd.color = glm::vec3(1,0,0);
                    
                    cmd.modelMatrix = model.u_model;

                    cmd.animated = model.animated;

                    if (cmd.animated)
                    {
                        cmd.boneTransforms = model.model->getReadBuffer();
                        cmd.boneInverseOffsets = model.model->boneInverseOffsets;
                    }

                    cmd.meshes = std::shared_ptr<std::vector<MeshVariant>>(&model.model->hitboxMeshes.value(), [](std::vector<MeshVariant>*) {});

                    return cmd; }));
            }
        }

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
            
            cmd.type = RenderType::OpaquePlane;

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
            
            cmd.type = RenderType::TransparentPlane;

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
            
            cmd.type = RenderType::Grid;

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

void Render::executeRender(::RenderBuffer &renderBuffer, bool toScreen)
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

    waterTimer = ShaderUtil::waterLoaded ? waterTimer += TimeManager::deltaTime : 0.0f;

    // If water loaded, render buffers
    if (waterTimer > 1 / SettingsManager::settings.video.waterFrameRate)
    {
        waterTimer = remainder(waterTimer, 1 / SettingsManager::settings.video.waterFrameRate);
        WaterPass = true;
        renderReflectRefract(renderBuffer.commandBuffer);
        WaterPass = false;
    }

    // Reset clip plane
    clipPlane = {0, 0, 0, 0};

    // Render rest of scene
    renderObjects(renderBuffer.commandBuffer);

    // Render debug menu
    if (SceneManager::engineState == EngineState::Running)
    {
        // Set debug data
        std::string debugText;
        FPS = (0.9f * FPS + 0.1f / TimeManager::deltaTime);

        // Select which debug renderer to use
        switch (SettingsManager::settings.debug.debugOverlay)
        {
        case debugOverlay::None:
            break;

        case debugOverlay::FPS:
            renderText(std::to_string(static_cast<int>(FPS)), 0.01f, 0.01f, 0.33f, debugColor);
            break;

        case debugOverlay::Physics:
            debugText = "Physics:\n";

            for (auto entry : debugPhysicsData)
            {
                debugText = debugText + entry.first + ": " + std::to_string(entry.second) + "\n";
            }

            renderText(debugText, 0.01f, 0.01f, 0.33f, debugColor);
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

        shader = ShaderUtil::load(shaderID::Post);
        shader->setInt("screenTexture", 0);
        shader->setBool("flipY", false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    Camera::cameraMoved = false;
    lastShader = nullptr;
}

void Render::renderText(std::string text, float x, float y, float scale, glm::vec3 color, float alpha, TextAlign textAlign)
{
    x *= WindowManager::screenUIScale * 2560.0f;
    y *= WindowManager::screenUIScale * 1440.0f;
    scale *= WindowManager::screenUIScale;

    float originalX = x;

    if (textAlign != TextAlign::Left)
    {
        float textWidth = calculateTextWidth(text, scale);
        if (textAlign == TextAlign::Center)
            x -= textWidth / 2.0f;
        else if (textAlign == TextAlign::Right)
            x -= textWidth;
    }

    // Load the shader for rendering text
    shader = ShaderUtil::load(shaderID::Text);

    if (shader != lastShader || WindowManager::windowSizeChanged)
    {
        // Set the projection matrix for the text shader
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WindowManager::screenWidth), static_cast<float>(WindowManager::screenHeight), 0.0f);
        shader->setMat4("projection", projection);

        lastShader = shader;
    }

    // Set text color uniform
    shader->setVec3("textColor", color.r, color.g, color.b);
    shader->setFloat("textAlpha", alpha);

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
    if (SceneManager::loadingState == loadingSteps.size())
    {
        statusString = "Finished Loading";
    }

    renderText(statusString, 0.05f, 0.05f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderText(progressString, 0.05f, 0.20f, 0.5f, glm::vec3(0.6f, 0.1f, 0.1f));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render::savePauseBackground()
{
    // Bind FBOs
    glBindFramebuffer(GL_READ_FRAMEBUFFER, Render::sceneFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copyFBO);

    // Attach target texture to copy FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pauseTexture, 0);

    // Check framebuffer completeness (optional but useful for debugging)
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Copy FBO is not complete!" << std::endl;

    // Perform blit
    glBlitFramebuffer(
        0, 0, WindowManager::screenWidth, WindowManager::screenHeight,
        0, 0, WindowManager::screenWidth, WindowManager::screenHeight,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::renderMenu(EngineState state)
{
    glDisable(GL_DEPTH_TEST);

    float alpha = std::clamp(UIManager::fade / UIManager::fadeTime, 0.0f, 1.0f);

    switch (state)
    {
    case EngineState::Title:
    case EngineState::TitleSettings:
    case EngineState::TestMenu:
    {
        float color = 0.0f;
        if (UIManager::shouldFadeBackground)
            color = easeInOutQuad(0.0f, 0.5f, alpha);
        else
            color = 0.5f;

        glClearColor(color, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        break;
    }
    case EngineState::Pause:
    case EngineState::Settings:
    {
        float darken = 0.0f;
        float darkenOffset = 0.0f;
        if (UIManager::shouldFadeBackground)
            if (UIManager::fadeToBlack)
            {
                darken = easeInOutQuad(1.0f, 0.5f, alpha);
                darkenOffset = easeInOutCirc(1.0f, 0.0f, alpha);
            }
            else
                darken = easeInOutQuad(0.0f, 0.5f, alpha);
        else
            darken = 0.5f;

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pauseTexture);

        shader = ShaderUtil::load(shaderID::DarkenBlur);
        shader->setInt("screenTexture", 0);
        shader->setVec2("texelSize", glm::vec2(1.0 / WindowManager::screenWidth, 1.0 / WindowManager::screenHeight));
        shader->setFloat("darkenAmount", darken);
        shader->setFloat("darkenPosition", 0.3f + darkenOffset);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    }

    float titleX = 0.02, titleY = 0.04;
    float shadowDistance = 0.003f;
    std::string titleText;

    switch (state)
    {
    case EngineState::Title:
        titleText = "Land Yachting Simulator";
        break;
    case EngineState::Pause:
        titleText = "Paused";
        break;
    case EngineState::TestMenu:
        titleText = "Tests";
        break;
    case EngineState::Settings:
    case EngineState::TitleSettings:
        titleText = "Settings";
        break;
    }

    float positionOffset = easeInOutQuad(-0.01f, 0.0f, alpha);

    renderText(titleText, titleX + positionOffset + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Left);
    renderText(titleText, titleX + positionOffset, titleY, 1.0f, glm::vec3(1.0f), alpha, TextAlign::Left);

    if (state == EngineState::Title)
    {
        glm::vec2 pos = {0.7f + positionOffset, 0.5f};

        renderImage("title-figure-black.png", pos + glm::vec2(0.005f, -0.01f), 835, 1024, alpha);
        renderImage("title-figure.png", pos, 835, 1024, alpha);
    }

    UIManager::render();

    glEnable(GL_DEPTH_TEST);
}