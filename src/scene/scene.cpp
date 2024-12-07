#include <scene/scene.h>
#include <file_manager/file_manager.h>

// Scene Constructor
Scene::Scene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model)
{
    loadScene(input_model, input_u_model);
};

// Scene Destructor
Scene::~Scene()
{
    models.clear(); // Clear the vector of pointers (no need to delete the objects, as they're owned by loadedModels)
    loadedModels.clear(); // Calls destructors of Model objects in the map
}

// Scene Renderer
void Scene::Draw(Shader &shader, glm::mat4 u_view, glm::mat4 u_projection)
{
    shader.use();
    // Apply view and projection to whole scene
    shader.setMat4("u_view", u_view);
    shader.setMat4("u_projection", u_projection);

    // Loop over scene models
    for (int i = 0; i < size(u_model); i++)
    {
        // Set model matrix for model and draw
        shader.setMat4("u_model", u_model[i]);
        shader.setMat4("u_normal", u_normal[i]);
        models[i]->Draw(shader);
    }
}

// Load models into scene
void Scene::loadScene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model)
{
    for (const std::string &path : input_model)
    {
        // If model not yet loaded
        if (loadedModels.find(path) == loadedModels.end())
        {
            // Load model
            loadedModels.emplace(path, FileManager::getPath(path));
        }

        // Push loaded path to model
        models.push_back(&loadedModels.at(path));
    }
    
    for (auto u_model_i : input_u_model)
    {
        u_model.push_back(u_model_i);
        u_normal.push_back(glm::transpose(glm::inverse(u_model_i)));
    }

}
