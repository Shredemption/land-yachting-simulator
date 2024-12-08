#ifndef SCENE_H
#define SCENE_H

#include <model/model.h>

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>

struct JSONModels
{
    std::string path;
    std::vector<float> scale;
    int angle;
    std::vector<float> rotationAxis;
    std::vector<float> translation;
};

struct JSONScene
{
    std::vector<JSONModels> models;
};

class Scene
{
public:
    // Scene Constructors
    Scene(std::string jsonPath);
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
    void loadModelToScene(JSONModels model);
};

#endif // SCENE_H