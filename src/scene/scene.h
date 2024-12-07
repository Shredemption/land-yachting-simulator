#ifndef SCENE_H
#define SCENE_H

#include <model/model.h>

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>

class Scene
{
public:
    // Scene Constructor
    Scene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model);

    // Scene Destructor
    ~Scene();

    // Scene Renderer
    void Draw(Shader &shader, glm::mat4 u_view, glm::mat4 u_projection);

private:
    std::vector<Model*> models;
    std::vector<glm::mat4> u_model;
    std::vector<glm::mat3> u_normal;

    std::unordered_map<std::string, Model> loadedModels;

    // Load models into scene
    void loadScene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model);
};

#endif // SCENE_H