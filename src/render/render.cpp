#include "render/render.h"
#include "event_handler/event_handler.h"
#include "frame_buffer/frame_buffer.h"
#include "camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::vec4 Render::clipPlane = glm::vec4(0, 0, 0, 0);

unsigned int Render::quadVAO = 0, Render::quadVBO = 0;
float Render::quadVertices[] = {0};
float Render::waterHeight = -0.5;

bool Render::WaterPass = true;
bool Render::debugMenu = false;
std::vector<std::pair<std::string, float>> Render::debugData;

FT_Library Render::ft;
FT_Face Render::face;
GLuint Render::textVAO, Render::textVBO;
GLuint Render::textTexture;
std::map<GLchar, Character> Render::Characters;
std::string Render::fontpath = "resources/fonts/Nexa-Heavy.ttf";

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
    renderSceneSkyBox(scene);

    if (Shader::waterLoaded && (EventHandler::frame % 2 == 0))
    {
        WaterPass = true;
        renderReflectRefract(scene, clipPlane);
        WaterPass = false;
    }

    clipPlane = {0, 0, 0, 0};
    renderSceneModels(scene, clipPlane);
    renderSceneUnitPlanes(scene, clipPlane);

    // renderTestQuad(FrameBuffer::reflectionFBO.colorTexture, 0, 0);
    // renderTestQuad(FrameBuffer::refractionFBO.colorTexture, 2 * EventHandler::screenWidth / 3, 0);

    if (debugMenu)
    {
        std::string debugText = "Debug Menu:\n";

        for (auto entry : debugData)
        {
            debugText = debugText + entry.first + ": " + std::to_string(entry.second) + "\n";
        }

        renderText(debugText, 10.0f, 10.0f, EventHandler::screenHeight / 2000.0f, glm::vec3(1.0f, 0.0f, 1.0f));

        debugData.clear();
    }



    Camera::cameraMoved = false;
}

void Render::renderSceneModels(Scene &scene, glm::vec4 clipPlane)
{
    for (auto model : scene.structModels)
    {
        Shader shader = Shader::load(model.shader);

        // Send light and view position to relevant shader
        shader.setVec3("lightPos", EventHandler::lightPos);
        shader.setVec3("viewPos", Camera::getPos());
        shader.setFloat("lightIntensity", EventHandler::lightInsensity);
        shader.setVec3("lightCol", EventHandler::lightCol);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", Camera::u_view);
        shader.setMat4("u_projection", Camera::u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", model.u_model);
        shader.setMat4("u_normal", model.u_normal);

        shader.setVec4("location_plane", clipPlane);

        if (model.model->boneHierarchy.size() > 0)
        {
            shader.setMat4Array("u_boneTransforms", model.model->boneTransforms);
            shader.setMat4Array("u_Offsets", model.model->boneOffsets);
            shader.setMat4Array("u_inverseOffsets", model.model->boneInverseOffsets);
        }

        renderModel(model);
    }
}

void Render::renderSceneUnitPlanes(Scene &scene, glm::vec4 clipPlane)
{
    // Render opaque planes
    for (auto unitPlane : scene.opaqueUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", Camera::u_view);
        shader.setMat4("u_projection", Camera::u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        shader.setVec4("location_plane", clipPlane);

        renderMesh(unitPlane.unitPlane);
    }

    // Sort transparent planes back to front based on distance from the camera
    std::sort(scene.transparentUnitPlanes.begin(), scene.transparentUnitPlanes.end(), [&](const UnitPlaneData &a, const UnitPlaneData &b)
              {
                  float distA = glm::distance(Camera::getPos(), a.position);
                  float distB = glm::distance(Camera::getPos(), b.position);
                  return distA > distB; // Sort by distance: farthest first, closest last
              });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render transparent planes
    for (auto unitPlane : scene.transparentUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", Camera::u_view);
        shader.setMat4("u_projection", Camera::u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        shader.setVec4("location_plane", clipPlane);

        if (unitPlane.shader == "water" & FrameBuffer::Water == false)
        {
            FrameBuffer::WaterFrameBuffers();
        }
        renderMesh(unitPlane.unitPlane);
    }
    glDisable(GL_BLEND);
}

void Render::renderSceneSkyBox(Scene &scene)
{
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);

    Shader shader = Shader::load("skybox");

    glBindVertexArray(scene.skyBox.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skyBox.textureID);

    shader.setMat4("u_view", glm::mat4(glm::mat3(Camera::u_view)));
    shader.setMat4("u_projection", Camera::u_projection);
    shader.setMat4("u_model", glm::rotate(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)), glm::vec3(0.0f, 0.0f, -0.175f)), glm::radians(EventHandler::sunAngle), glm::vec3(0.0f, 0.0f, 1.0f)));

    shader.setInt("skybox", 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
}

void Render::renderModel(ModelData model)
{
    for (unsigned int i = 0; i < model.model->meshes.size(); i++)
    {
        Render::renderMesh(model.model->meshes[i]);
    }
}

// Render mesh
void Render::renderMesh(Mesh mesh)
{
    if (mesh.shader == "pbr")
    {
        renderPBR(mesh);
    }
    else if (mesh.shader == "default")
    {
        renderDefault(mesh);
    }
    else if (mesh.shader == "simple")
    {
        renderSimple(mesh);
    }
    else if (mesh.shader == "water")
    {
        if (!WaterPass)
        {
            renderWater(mesh);
        }
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

void Render::renderPBR(Mesh mesh) // FIXME : Not showing when rendered (but is shown wrong in reflection of water fsr)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int aoNr = 1;

    Shader shader = Shader::load("pbr");

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
        shader.setInt(("material." + name + number).c_str(), i);
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

void Render::renderWater(Mesh mesh)
{
    // Load surface textures
    Texture DuDv = LoadStandaloneTexture("waterDUDV.png");
    Texture normal = LoadStandaloneTexture("waterNormal.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FrameBuffer::reflectionFBO.colorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, FrameBuffer::refractionFBO.colorTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, DuDv.id);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normal.id);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, FrameBuffer::refractionFBO.depthTexture);

    Shader shader = Shader::load("water");
    shader.setInt("reflectionTexture", 0);
    shader.setInt("refractionTexture", 1);
    shader.setInt("dudvMap", 2);
    shader.setInt("normalMap", 3);
    shader.setInt("depthMap", 4);
    shader.setFloat("moveOffset", EventHandler::time);
    shader.setVec3("cameraPosition", Camera::getPos());
    shader.setVec3("lightPos", EventHandler::lightPos);
    shader.setVec3("lightCol", EventHandler::lightCol);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw Mesh
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void Render::renderReflectRefract(Scene &scene, glm::vec4 clipPlane)
{
    // ===== REFLECTOIN =====
    // Bind reflection buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::reflectionFBO);

    clipPlane = {0, 0, 1, -waterHeight};
    Camera::setCamDirection(glm::vec3(-Camera::getRotation()[0], Camera::getRotation()[1], Camera::getRotation()[2]));
    float distance = 2 * (Camera::getPos()[2] - waterHeight);
    Camera::genViewMatrix(Camera::getPos() + glm::vec3(0, 0, -distance));

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox(scene);
    renderSceneModels(scene, clipPlane);
    renderSceneUnitPlanes(scene, clipPlane);

    // ===== REFRACTION =====
    // Bind refraction buffer
    FrameBuffer::bindFrameBuffer(FrameBuffer::refractionFBO);

    clipPlane = {0, 0, -1, waterHeight};
    Camera::setCamDirection(Camera::getRotation());
    Camera::genViewMatrix(Camera::getPos());

    // Draw to it
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderSceneSkyBox(scene);
    renderSceneModels(scene, clipPlane);
    renderSceneUnitPlanes(scene, clipPlane);

    // Unbind buffers, bind default one
    FrameBuffer::unbindCurrentFrameBuffer();
}

void Render::renderTestQuad(GLuint texture, int x, int y)
{
    glViewport(x, y, EventHandler::screenWidth / 3, EventHandler::screenHeight / 3);

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

Texture Render::LoadStandaloneTexture(std::string fileName)
{
    Texture loadTexture;
    // If texture already loaded
    if (Model::textureCache.find(fileName) != Model::textureCache.end())
    {
        // Use cached texture
        loadTexture = Model::textureCache[fileName].texture;
        Model::textureCache[fileName].refCount++;
    }
    else
    {
        // Define and load new texture to texture cache
        Texture texture;
        texture.id = Model::TextureFromFile(fileName.c_str(), "../resources/textures");
        texture.type = "standalone";
        texture.path = fileName.c_str();

        loadTexture = texture;
        Model::textureCache[fileName].texture = texture;
    }

    return loadTexture;
}

void Render::initFreeType(std::string &fontPath)
{
    // Initialize FreeType
    if (FT_Init_FreeType(&ft))
        std::cerr << "ERROR: Could not initialize FreeType\n";

    fontPath = "../" + fontPath;

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

    for (GLuint c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR: Failed to load Glyph\n";
            continue;
        }

        // Check if the character fits in the current line
        if (xPos + face->glyph->bitmap.width >= atlasWidth)
        {
            xPos = 0;
            yPos += face->glyph->bitmap.rows;
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
        xPos += face->glyph->bitmap.width;
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
    // Load the shader for rendering text
    Shader shader = Shader::load("text");

    // Set text color uniform
    glUniform3f(glGetUniformLocation(shader.m_id, "textColor"), color.x, color.y, color.z);

    // Set the projection matrix for the text shader
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(EventHandler::screenWidth), static_cast<float>(EventHandler::screenHeight), 0.0f);
    shader.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    glBindVertexArray(textVAO);

    // Enable blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float startX = x;                                          // Store the initial x position
    float lineSpacing = Characters['H'].Size.y * scale * 1.2f; // Adjust line spacing with a small padding

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