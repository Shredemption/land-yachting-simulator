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

JSONCONS_N_MEMBER_TRAITS(JSONModel, 1, path, scale, angle, rotationAxis, translation, shader, animated, controlled);
JSONCONS_N_MEMBER_TRAITS(JSONUnitPlane, 0, color, scale, angle, rotationAxis, translation, shader);
JSONCONS_N_MEMBER_TRAITS(JSONSkybox, 6, up, down, left, right, front, back);
JSONCONS_N_MEMBER_TRAITS(JSONScene, 0, models, unitPlanes, skyBox, bgColor);

// TODO: textured unitplane
// TODO: environment

Scene::Scene(std::string jsonPath)
{
    const std::string path = "../" + jsonPath;

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

    // For each model in scene
    for (JSONModel model : jsonScene.models)
    {
        loadModelToScene(model);
    }

    for (JSONUnitPlane unitPlane : jsonScene.unitPlanes)
    {
        loadUnitPlaneToScene(unitPlane);
    }

    for (JSONSkybox skybox : jsonScene.skyBox)
    {
        loadSkyBoxToScene(skybox);
    }

    bgColor = glm::vec3(jsonScene.bgColor[0], jsonScene.bgColor[1], jsonScene.bgColor[2]);
};

Scene::~Scene()
{
}

void Scene::loadModelToScene(JSONModel model)
{
    // Setup empty structModel unit
    ModelData loadModel;

    // Find model location using map
    std::string modelPath = Model::modelMap[model.path];

    // If model not yet loaded
    if (loadedModels.find(modelPath) == loadedModels.end())
    {
        // Load model with path and shader name
        loadedModels.emplace(modelPath, std::make_pair(FileManager::getPath(modelPath), model.shader));
    }

    // Push loaded path to model
    loadModel.model = &loadedModels.at(modelPath);

    // Generate u_model
    glm::mat4 u_model_i = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(model.translation[0], model.translation[1], model.translation[2])),
            glm::radians((float)model.angle),
            glm::vec3(model.rotationAxis[0], model.rotationAxis[1], model.rotationAxis[2])),
        glm::vec3(model.scale[0], model.scale[1], model.scale[2]));

    loadModel.u_model = u_model_i;
    loadModel.u_normal = glm::transpose(glm::inverse(u_model_i));

    loadModel.shader = model.shader;

    loadModel.animated = model.animated;
    loadModel.controlled = model.controlled;

    structModels.push_back(loadModel);
}

void Scene::loadUnitPlaneToScene(JSONUnitPlane unitPlane)
{
    UnitPlaneData loadUnitPlane;

    loadUnitPlane.color = {unitPlane.color[0], unitPlane.color[1], unitPlane.color[2]};
    loadUnitPlane.shader = unitPlane.shader;

    loadUnitPlane.unitPlane = Mesh::genUnitPlane(loadUnitPlane.color, loadUnitPlane.shader);

    // Generate u_model
    glm::mat4 u_model_i = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(unitPlane.translation[0], unitPlane.translation[1], unitPlane.translation[2])),
            glm::radians((float)unitPlane.angle),
            glm::vec3(unitPlane.rotationAxis[0], unitPlane.rotationAxis[1], unitPlane.rotationAxis[2])),
        glm::vec3(unitPlane.scale[0], unitPlane.scale[1], unitPlane.scale[2]));

    loadUnitPlane.u_model = u_model_i;
    loadUnitPlane.u_normal = glm::transpose(glm::inverse(u_model_i));

    loadUnitPlane.position = u_model_i[3];

    if (loadUnitPlane.isTransparent())
        transparentUnitPlanes.push_back(loadUnitPlane);
    else
        opaqueUnitPlanes.push_back(loadUnitPlane);
}

void Scene::loadSkyBoxToScene(JSONSkybox loadSkyBox)
{
    this->skyBox[0].up = loadSkyBox.up;
    this->skyBox[0].down = loadSkyBox.down;
    this->skyBox[0].left = loadSkyBox.left;
    this->skyBox[0].right = loadSkyBox.right;
    this->skyBox[0].front = loadSkyBox.front;
    this->skyBox[0].back = loadSkyBox.back;

    this->skyBox[0].textureID = Model::LoadSkyBoxTexture(this->skyBox[0]);
    this->skyBox[0].VAO = Mesh::setupSkyBoxMesh();
}