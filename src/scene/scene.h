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
    std::unordered_map<std::string, Model> loadedModels;

    Scene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model);

    void Draw(Shader &shader, glm::mat4 u_view, glm::mat4 u_projection);

private:
    std::vector<Model*> models;
    std::vector<glm::mat4> u_model;
    std::vector<glm::mat3> u_normal;

    void loadScene(std::vector<std::string> input_model, std::vector<glm::mat4> input_u_model);
};

#endif // SCENE_H