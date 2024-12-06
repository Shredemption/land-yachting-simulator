#ifndef SCENE_H
#define SCENE_H

#include <model/model.h>

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
using namespace std;

class Scene
{
public:
    std::unordered_map<std::string, Model> loadedModels;

    Scene(vector<string> input_model, vector<glm::mat4> input_u_model);

    void Draw(Shader &shader, glm::mat4 u_view, glm::mat4 u_projection);

private:
    vector<Model*> models;
    vector<glm::mat4> u_model;

    void loadScene(vector<string> input_model, vector<glm::mat4> input_u_model);
};

#endif // SCENE_H