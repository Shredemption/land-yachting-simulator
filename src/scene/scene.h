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

struct ModelData
{
    Model *model;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    std::string shader;
};

struct UnitPlaneData
{
    glm::vec3 color;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    std::string shader;
    Mesh unitPlane = Mesh::genUnitPlane(color, shader);

    // Data from transparent rendering
    glm::vec3 position;
    bool isTransparent() const
    {
        // Add logic to check whether the shader is transparent or not
        return (shader == "water");
    }
};

class Scene
{
public:
    // Scene Constructors
    Scene(std::string jsonPath);

    // Scene Destructor
    ~Scene();

    // Model data
    std::vector<ModelData> structModels;
    std::unordered_map<std::string, Model> loadedModels;

    // Unit Plane Data
    std::vector<UnitPlaneData> transparentUnitPlanes;
    std::vector<UnitPlaneData> opaqueUnitPlanes;

private:
    // Load models into scene
    void loadModelToScene(JSONModels model);
    void loadUnitPlaneToScene(JSONUnitPlane unitPlane);
};

#endif