#ifndef SCENE_H
#define SCENE_H

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "model.h"

class Scene
{
public:
    std::unordered_map<std::string, Model> loadedModels;

    Scene(vector<string> input_model, vector<glm::mat4> input_u_model)
    {
        loadScene(input_model, input_u_model);
    };

    void Draw(Shader &shader, glm::mat4 u_view, glm::mat4 u_projection)
    {
        // Apply view and projection to whole scene
        shader.setMat4("u_view", u_view);
        shader.setMat4("u_projection", u_projection);

        // Loop over scene models
        for (int i = 0; i < size(u_model); i++)
        {
            // Set model matrix for model and draw
            shader.setMat4("u_model", u_model[i]);
            shader.use();
            model[i].Draw(shader);
        }
    }

private:
    vector<Model*> models;
    vector<glm::mat4> u_model;

    void loadScene(vector<string> input_model, vector<glm::mat4> input_u_model)
    {
        for (const string& path : input_model)
        {
            // If model not yet loaded
            if (loadedModels.find(path) == loadedModels.end())
            {
                // Load model
                loadedModels.emplace(path, Model(FileManager::getPath(path)));
            }

            // Push loaded path to model
            models.push_back(&loadedModels.at(path));
        }
        u_model = std::move(input_u_model);
    }
};

#endif // SCENE_H