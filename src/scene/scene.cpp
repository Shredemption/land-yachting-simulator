#include <scene/scene.h>
#include <file_manager/file_manager.h>
#include <filesystem>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_traits_macros.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model/model.h"
#include "mesh/mesh.h"
#include "event_handler/event_handler.h"

JSONCONS_ALL_MEMBER_TRAITS(JSONModels, path, scale, angle, rotationAxis, translation, shader);
JSONCONS_ALL_MEMBER_TRAITS(JSONUnitPlane, color, scale, angle, rotationAxis, translation, shader);
JSONCONS_ALL_MEMBER_TRAITS(JSONScene, models, unitPlanes);

// Scene Constructor
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

    // Parse the JSON
    jsoncons::json jsonScene;
    try
    {
        jsonScene = jsoncons::json::parse(file);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    // Parse json into struct
    JSONScene jsonModels = jsonScene.as<JSONScene>();

    // For each model in scene
    for (JSONModels model : jsonModels.models)
    {
        loadModelToScene(model);
    }

    for (JSONUnitPlane unitPlane : jsonModels.unitPlanes)
    {
        loadUnitPlaneToScene(unitPlane);
    }
};

// Scene Destructor
Scene::~Scene()
{
}

// Scene Renderer
void Scene::Draw(glm::mat4 u_view, glm::mat4 u_projection)
{
    Scene::DrawModels(u_view, u_projection);
    Scene::DrawUnitPlanes(u_view, u_projection);
}

void Scene::DrawModels(glm::mat4 u_view, glm::mat4 u_projection)
{
    for (auto model : structModels)
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

        model.model->Draw();
    }
}

void Scene::DrawUnitPlanes(glm::mat4 u_view, glm::mat4 u_projection)
{
    // Render opaque planes
    for (auto unitPlane : opaqueUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        unitPlane.unitPlane.Draw();
    }

    // Sort transparent planes back to front based on distance from the camera
    std::sort(transparentUnitPlanes.begin(), transparentUnitPlanes.end(), [&](const UnitPlaneData& a, const UnitPlaneData& b) {
        float distA = glm::distance(EventHandler::cameraPosition, a.position);
        float distB = glm::distance(EventHandler::cameraPosition, b.position);
        return distA > distB;  // Sort by distance: farthest first, closest last
    });

    // Render transparent planes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto unitPlane : transparentUnitPlanes)
    {
        Shader shader = Shader::load(unitPlane.shader);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", unitPlane.u_model);
        shader.setMat4("u_normal", unitPlane.u_normal);

        unitPlane.unitPlane.Draw();
    }

    glDisable(GL_BLEND);

}

// Load models into scene
void Scene::loadModelToScene(JSONModels model)
{
    // Setup empty structModel unit
    ModelData loadModel;

    // Find model location using map
    Model::modelMap[model.path];

    // If model not yet loaded
    if (loadedModels.find(Model::modelMap[model.path]) == loadedModels.end())
    {
        // Load model
        loadedModels.emplace(Model::modelMap[model.path], FileManager::getPath(Model::modelMap[model.path]));
    }

    // Push loaded path to model
    loadModel.model = &loadedModels.at(Model::modelMap[model.path]);

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

    structModels.push_back(loadModel);
}

void Scene::loadUnitPlaneToScene(JSONUnitPlane unitPlane)
{
    UnitPlaneData loadUnitPlane;

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

    loadUnitPlane.shader = unitPlane.shader;
    loadUnitPlane.position = u_model_i[3];

    if (loadUnitPlane.isTransparent())
        transparentUnitPlanes.push_back(loadUnitPlane);
    else
        opaqueUnitPlanes.push_back(loadUnitPlane);
}