#include <scene/scene.h>
#include <file_manager/file_manager.h>
#include <filesystem>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_traits_macros.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model/model.h"

JSONCONS_ALL_MEMBER_TRAITS(JSONModels, path, scale, angle, rotationAxis, translation, shader);
JSONCONS_ALL_MEMBER_TRAITS(JSONScene, models)

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
};

// Scene Destructor
Scene::~Scene()
{
}

// Scene Renderer
void Scene::Draw(glm::mat4 u_view, glm::mat4 u_projection)
{
    Scene::DrawModels(u_view, u_projection);
}

void Scene::DrawModels(glm::mat4 u_view, glm::mat4 u_projection)
{
    for (int i = 0; i < size(models); i++)
    {
        Shader shader = Shader::load(models_shaders[i]);

        // Send light and view position to relevant shader
        shader.setVec3("lightPos", EventHandler::lightPos);
        shader.setVec3("viewPos", EventHandler::cameraPosition);
        shader.setFloat("lightIntensity", 2.0f);
        shader.setVec3("lightCol", 1.f, 1.f, 1.f);

        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Set model matrix for model and draw
        shader.setMat4("u_model", models_u_model[i]);
        shader.setMat4("u_normal", models_u_normal[i]);

        models[i]->Draw();
    }
}
    {
        // Set model matrix for model and draw
        shader.setMat4("u_model", u_model[i]);
        shader.setMat4("u_normal", u_normal[i]);
        models[i]->Draw(shader);
    }
}

// Load models into scene
void Scene::loadModelToScene(JSONModels model)
{
    // Find model location using map
    Model::modelMap[model.path];

    // If model not yet loaded
    if (loadedModels.find(Model::modelMap[model.path]) == loadedModels.end())
    {
        // Load model
        loadedModels.emplace(Model::modelMap[model.path], FileManager::getPath(Model::modelMap[model.path]));
    }

    // Push loaded path to model
    models.push_back(&loadedModels.at(Model::modelMap[model.path]));

    // Generate u_model
    glm::mat4 u_model_i = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(model.translation[0], model.translation[1], model.translation[2])),
            glm::radians((float)model.angle),
            glm::vec3(model.rotationAxis[0], model.rotationAxis[1], model.rotationAxis[2])),
        glm::vec3(model.scale[0], model.scale[1], model.scale[2]));

    models_u_model.push_back(u_model_i);
    models_u_normal.push_back(glm::transpose(glm::inverse(u_model_i)));

    models_shaders.push_back(model.shader);
}
