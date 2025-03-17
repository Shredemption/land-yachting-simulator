#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>

#include "model/model.h"

struct JSONModel
{
    std::string path = "none";
    std::vector<float> scale = {1, 1, 1};
    float angle = 0;
    std::vector<float> rotationAxis = {0, 1, 0};
    std::vector<float> translation = {0, 0, 0};
    std::string shader = "default";
    bool animated = false;
    bool controlled = false;
};

struct JSONUnitPlane
{
    std::vector<float> color = {1, 1, 1};
    std::vector<float> scale = {1, 1, 1};
    float angle = 0;
    std::vector<float> rotationAxis = {0, 1, 0};
    std::vector<float> translation = {0, 0, 0};
    std::string shader = "simple";
};

struct JSONSkybox
{
    std::string up = "";
    std::string down = "";
    std::string left = "";
    std::string right = "";
    std::string front = "";
    std::string back = "";
};

struct JSONScene
{
    std::vector<JSONModel> models = {};
    std::vector<JSONUnitPlane> unitPlanes = {};
    JSONSkybox skyBox = JSONSkybox();
};

class Physics;

struct ModelData
{
    Model *model;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    std::string shader;
    bool animated;
    bool controlled;
    std::vector<Physics *> physics;
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
        return (shader == "water" | shader == "toon-water");
    }
};

struct SkyBoxData
{
    std::string up = "";
    std::string down = "";
    std::string left = "";
    std::string right = "";
    std::string front = "";
    std::string back = "";
    unsigned int textureID;
    unsigned int VAO;
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

    // Skybox
    SkyBoxData skyBox;

private:
    // Load models into scene
    void loadModelToScene(JSONModel model);
    void loadUnitPlaneToScene(JSONUnitPlane unitPlane);
    void loadSkyBoxToScene(JSONSkybox skyBox);
};

#endif