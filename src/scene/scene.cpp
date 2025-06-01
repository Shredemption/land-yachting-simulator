#include <scene/scene.h>

#include <filesystem>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_traits_macros.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model/model.h"
#include "mesh/mesh.h"
#include "event_handler/event_handler.h"
#include "frame_buffer/frame_buffer.h"
#include "file_manager/file_manager.h"
#include "scene_manager/scene_manager.h"
#include "texture_manager/texture_manager.h"

// Json mappings
JSONCONS_N_MEMBER_TRAITS(JSONModel, 1, name, scale, angle, rotationAxis, translation, shader, animated, controlled);
JSONCONS_N_MEMBER_TRAITS(JSONUnitPlane, 0, color, scale, angle, rotationAxis, translation, shader);
JSONCONS_N_MEMBER_TRAITS(JSONGrid, 0, gridSize, scale, lod, color, angle, rotationAxis, translation, shader);
JSONCONS_N_MEMBER_TRAITS(JSONSkybox, 6, up, down, left, right, front, back);
JSONCONS_N_MEMBER_TRAITS(JSONText, 1, text, color, position, scale);
JSONCONS_N_MEMBER_TRAITS(JSONImage, 2, file, size, position, scale, rotation, mirrored);
JSONCONS_N_MEMBER_TRAITS(JSONScene, 0, models, unitPlanes, grids, skyBox, texts, images, bgColor);

Scene::Scene(std::string jsonPath, std::string sceneName)
{
    const std::string path = "../" + jsonPath;
    this->name = sceneName;

    // Check if the file exists
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File not found: " + path);
    }

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path);
    }

    // Parse json into struct
    JSONScene jsonScene = jsoncons::decode_json<JSONScene>(file);

    SceneManager::loadingState++;

    // Set background color from scene
    bgColor = glm::vec3(jsonScene.bgColor[0], jsonScene.bgColor[1], jsonScene.bgColor[2]);

    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = jsonScene.texts.size();

    // Load texts from scene
    for (JSONText text : jsonScene.texts)
    {
        loadTextToScene(text);
        SceneManager::loadingProgress.first++;
    }

    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = jsonScene.images.size();

    // Load texts from scene
    for (JSONImage image : jsonScene.images)
    {
        loadImageToScene(image);
        SceneManager::loadingProgress.first++;
    }

    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = jsonScene.models.size();

    // Load models from scene
    for (JSONModel model : jsonScene.models)
    {
        loadModelToScene(model);
        SceneManager::loadingProgress.first++;
    }

    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = jsonScene.unitPlanes.size();

    // Load unitplanes from scene
    for (JSONUnitPlane unitPlane : jsonScene.unitPlanes)
    {
        loadUnitPlaneToScene(unitPlane);
        SceneManager::loadingProgress.first++;
    }

    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = jsonScene.grids.size();

    // Generate Grids form scene
    for (JSONGrid grid : jsonScene.grids)
    {
        for (int i = 0; i <= grid.lod; i++)
        {
            JSONGrid loadGrid = grid;
            loadGrid.lod = i;
            loadGridToScene(loadGrid);
        }
        SceneManager::loadingProgress.first++;
    }

    SceneManager::loadingState++;

    // Load skybox from scene
    hasSkyBox = false;
    for (JSONSkybox skybox : jsonScene.skyBox)
    {
        loadSkyBoxToScene(skybox);
    }

    int texLoadCount = TextureManager::textureQueue.size();
    for (const auto &arr : TextureManager::textureArrays)
        texLoadCount += arr.second.pendingTextures.size();
    SceneManager::loadingState++;
    SceneManager::loadingProgress.first = 0;
    SceneManager::loadingProgress.second = texLoadCount;

    TextureManager::loadQueuedPixelData();

    SceneManager::loadingState++;
};

void Scene::loadModelToScene(JSONModel model)
{
    // Setup empty structModel unit
    ModelData loadModel;

    // Find model location using map
    auto &modelEntry = Model::modelMap[model.name];

    ModelType modelType = ModelType::mtModel;

    if (modelEntry.type == "yacht")
    {
        modelType = ModelType::mtYacht;
    }

    std::vector<std::string> paths = {FileManager::getPath(modelEntry.mainPath)};

    for (auto lodPath : modelEntry.lodPaths)
    {
        paths.push_back(FileManager::getPath(lodPath));
    }

    // If model not yet loaded
    if (loadedModels.find(modelEntry.mainPath) == loadedModels.end())
    {
        // Load model with path and shader name
        loadedModels.emplace(modelEntry.mainPath, std::make_tuple(model.name, paths, Shader::ShaderFromName(model.shader), modelType));
    }

    // Push loaded path to model
    loadModel.model = &loadedModels.at(modelEntry.mainPath);

    // Generate u_model
    glm::mat4 u_model_i = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(model.translation[0], model.translation[1], model.translation[2])),
            glm::radians((float)model.angle),
            glm::vec3(model.rotationAxis[0], model.rotationAxis[1], model.rotationAxis[2])),
        glm::vec3(model.scale[0], model.scale[1], model.scale[2]));

    // Model and normal matrices
    loadModel.u_model = u_model_i;
    loadModel.u_normal = glm::transpose(glm::inverse(u_model_i));

    // Model shader
    loadModel.shader = Shader::ShaderFromName(model.shader);

    // Model animation data
    loadModel.animated = model.animated;
    loadModel.controlled = model.controlled;

    // Save model
    this->structModels.emplace_back(std::move(loadModel));

    // If model is a yacht, save name to yachts list
    if (Model::modelMap[model.name].type == "yacht")
    {
        loadedYachts.push_back(model.name);
    }
}

void Scene::loadUnitPlaneToScene(JSONUnitPlane unitPlane)
{
    // Empty unitplane for loading
    UnitPlaneData loadUnitPlane;

    // Save color and shader
    loadUnitPlane.color = glm::vec3(unitPlane.color[0], unitPlane.color[1], unitPlane.color[2]);
    loadUnitPlane.shader = Shader::ShaderFromName(unitPlane.shader);

    // Generate mesh from color and shader
    if (loadUnitPlane.shader == shaderID::shSimple)
        loadUnitPlane.unitPlane = Mesh<VertexSimple>::genUnitPlane(loadUnitPlane.color, loadUnitPlane.shader);
    else
        loadUnitPlane.unitPlane = Mesh<VertexTextured>::genUnitPlane(loadUnitPlane.color, loadUnitPlane.shader);

    // Generate u_model
    glm::mat4 u_model_i = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(unitPlane.translation[0], unitPlane.translation[1], unitPlane.translation[2])),
            glm::radians((float)unitPlane.angle),
            glm::vec3(unitPlane.rotationAxis[0], unitPlane.rotationAxis[1], unitPlane.rotationAxis[2])),
        glm::vec3(unitPlane.scale[0], unitPlane.scale[1], unitPlane.scale[2]));

    // Save model and normal matrices
    loadUnitPlane.u_model = u_model_i;
    loadUnitPlane.u_normal = glm::transpose(glm::inverse(u_model_i));

    // Pull position data form u_model
    loadUnitPlane.position = u_model_i[3];

    // Push unitplane to relevant planelist
    if (loadUnitPlane.isTransparent())
    {
        this->transparentUnitPlanes.push_back(loadUnitPlane);
    }
    else
    {
        this->opaqueUnitPlanes.push_back(loadUnitPlane);
    }

    if (loadUnitPlane.shader == shaderID::shWater)
    {
        TextureManager::queueTextureToArrayByFilename("waterDUDV.png", "waterTextureArray");
        TextureManager::queueTextureToArrayByFilename("waterNormal.png", "waterTextureArray");
    }

    if (loadUnitPlane.shader == shaderID::shToonWater)
    {
        TextureManager::queueStandaloneTexture("toonWater.jpeg");
        TextureManager::queueStandaloneTexture("waterNormal.png");
        TextureManager::queueStandaloneTexture("heightmap.jpg");
    }
}

void Scene::loadGridToScene(JSONGrid grid)
{
    // Empty grid for loading
    GridData loadGrid;

    // Save grid data
    loadGrid.color = glm::vec3(grid.color[0], grid.color[1], grid.color[2]);
    loadGrid.shader = Shader::ShaderFromName(grid.shader);
    loadGrid.lod = grid.lod;
    loadGrid.gridSize = glm::vec2(grid.gridSize[0], grid.gridSize[1]);

    // Generate grid mesh
    if (grid.shader == "toon-terrain")
        loadGrid.grid = Mesh<VertexSimple>::genGrid(loadGrid.gridSize[0], loadGrid.gridSize[1], loadGrid.lod, loadGrid.color, loadGrid.shader);
    else
        loadGrid.grid = Mesh<VertexTextured>::genGrid(loadGrid.gridSize[0], loadGrid.gridSize[1], loadGrid.lod, loadGrid.color, loadGrid.shader);

    // Generate u_model
    glm::mat4 u_model_i =
        glm::rotate(
            glm::scale(
                glm::translate(
                    glm::mat4(1.0f),
                    glm::vec3(grid.translation[0], grid.translation[1], grid.translation[2])),
                glm::vec3(grid.scale * pow(2, loadGrid.lod), grid.scale * pow(2, loadGrid.lod), 1.0f)),
            glm::radians((float)grid.angle), glm::vec3(grid.rotationAxis[0], grid.rotationAxis[1], grid.rotationAxis[2]));

    // Save model and normal matrices
    loadGrid.u_model = u_model_i;
    loadGrid.u_normal = glm::transpose(glm::inverse(u_model_i));

    // Push loaded grid to scene
    this->grids.push_back(loadGrid);

    if (loadGrid.shader == shaderID::shToonTerrain)
    {
        TextureManager::queueStandaloneTexture("heightmap.jpg");
    }
}

void Scene::loadSkyBoxToScene(JSONSkybox loadSkyBox)
{
    // Track if scene has skybox
    hasSkyBox = true;

    // Load skybox texture names
    this->skyBox.up = loadSkyBox.up;
    this->skyBox.down = loadSkyBox.down;
    this->skyBox.left = loadSkyBox.left;
    this->skyBox.right = loadSkyBox.right;
    this->skyBox.front = loadSkyBox.front;
    this->skyBox.back = loadSkyBox.back;
}

void Scene::loadTextToScene(JSONText text)
{
    // Empty text for loading
    TextData loadText;

    // Save text data
    loadText.text = text.text;
    loadText.color = glm::vec3(text.color[0], text.color[1], text.color[2]);
    loadText.position = glm::vec2(text.position[0], text.position[1]);
    loadText.scale = text.scale;

    // Push text to scene
    this->texts.push_back(loadText);
}

void Scene::loadImageToScene(JSONImage image)
{
    // Empty text for loading
    ImageData loadImage;

    loadImage.file = image.file;
    loadImage.mirrored = image.mirrored;
    loadImage.rotation = image.rotation;
    loadImage.position = glm::vec2(image.position[0], image.position[1]);
    loadImage.scale = glm::vec2(image.scale[0], image.scale[1]);
    loadImage.width = image.size[0];
    loadImage.height = image.size[1];

    TextureManager::queueStandaloneImage(image.file);

    // Push text to scene
    this->images.push_back(loadImage);
}

void Scene::uploadToGPU()
{
    TextureManager::uploadToGPU();
    // For each type, upload data to opengl context
    for (auto &modelData : structModels)
    {
        modelData.model->uploadToGPU();
    }
    for (auto &transparentUnitPlane : transparentUnitPlanes)
    {
        std::visit([](auto &mesh)
                   { mesh.uploadToGPU(); },
                   transparentUnitPlane.unitPlane);
    }
    for (auto &opaqueUnitPlane : opaqueUnitPlanes)
    {
        std::visit([](auto &mesh)
                   { mesh.uploadToGPU(); },
                   opaqueUnitPlane.unitPlane);
    }
    for (auto &grid : grids)
    {
        std::visit([](auto &mesh)
                   { mesh.uploadToGPU(); },
                   grid.grid);
    }
    for (auto &image : images)
    {
        image.textureID = TextureManager::getStandaloneTextureID(image.file);
    }
    if (hasSkyBox)
    {
        this->skyBox.textureID = TextureManager::loadSkyboxTexture(this->skyBox);
        this->skyBox.VAO = Mesh<VertexSkybox>::setupSkyBoxMesh();
    }
}