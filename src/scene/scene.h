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
    std::string shader;
};

struct JSONUnitPlane
{
    std::vector<float> color;
    std::vector<float> scale;
    int angle;
    std::vector<float> rotationAxis;
    std::vector<float> translation;
    std::string shader;
};

struct JSONScene
{
    std::vector<JSONModels> models;
    std::vector<JSONUnitPlane> unitPlanes;
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
    void Draw(glm::mat4 u_view, glm::mat4 u_projection);
    void DrawModels(glm::mat4 u_view, glm::mat4 u_projection);
    void DrawUnitPlanes(glm::mat4 u_view, glm::mat4 u_projection);

private:
    // Model data
    std::vector<Model*> models;
    std::vector<glm::mat4> models_u_model;
    std::vector<glm::mat3> models_u_normal;
    std::unordered_map<std::string, Model> loadedModels;
    std::vector<std::string> models_shaders;

    // Unit Plane Data
    std::vector<Mesh> unitPlanes;
    std::vector<glm::mat4> unitPlanes_u_model;
    std::vector<glm::mat3> unitPlanes_u_normal;
    std::vector<std::string> unitPlanes_shaders;

    // Load models into scene
    void loadModelToScene(JSONModels model);
    void loadUnitPlaneToScene(JSONUnitPlane unitPlane);
};

#endif