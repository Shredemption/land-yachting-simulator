#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>

#include "model/model.h"

struct JSONModel
{
    std::string name = "none";
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

struct JSONGrid
{
    std::vector<float> gridSize = {1, 1};
    float scale = 1;
    float lod = 0;
    std::vector<float> color = {1, 1, 1};
    float angle = 0;
    std::vector<float> rotationAxis = {0, 0, 1};
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

struct JSONText
{
    std::string text = "";
    std::vector<float> color = {1, 1, 1};
    std::vector<float> position = {0, 0};
    float scale = 1;
};

struct JSONScene
{
    std::vector<JSONModel> models = {};
    std::vector<JSONUnitPlane> unitPlanes = {};
    std::vector<JSONGrid> grids = {};
    std::vector<JSONSkybox> skyBox = {};
    std::vector<JSONText> texts = {};
    std::vector<float> bgColor = {0, 0, 0};
};

class Physics;

struct ModelData
{
    Model *model;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    shaderID shader;
    bool animated;
    bool controlled;
    std::vector<Physics *> physics;
};

struct UnitPlaneData
{
    glm::vec3 color;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    shaderID shader;
    MeshVariant unitPlane = Mesh<VertexTextured>::genUnitPlane(color, shader);

    // Data from transparent rendering
    glm::vec3 position;
    bool isTransparent() const
    {
        // Add logic to check whether the shader is transparent or not
        return (shader == shaderID::shWater);
    }
};

struct GridData
{
    glm::vec3 color;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    shaderID shader;
    glm::vec2 gridSize;
    float lod;
    MeshVariant grid = Mesh<VertexTextured>::genGrid(gridSize.x, gridSize.y, lod, color, shader);
};

struct SkyBoxData
{
    std::string up;
    std::string down;
    std::string left;
    std::string right;
    std::string front;
    std::string back;
    unsigned int textureID;
    unsigned int VAO;
};

struct TextData
{
    std::string text;
    glm::vec3 color;
    glm::vec2 position;
    float scale;
};

class Scene
{
public:
    Scene(std::string jsonPath, std::string sceneName);
    void uploadToGPU();

    // Local scene data
    std::string name;
    std::vector<ModelData> structModels;
    std::unordered_map<std::string, Model> loadedModels;
    std::vector<std::string> loadedYachts;
    std::vector<UnitPlaneData> transparentUnitPlanes;
    std::vector<UnitPlaneData> opaqueUnitPlanes;
    std::vector<GridData> grids;
    SkyBoxData skyBox;
    bool hasSkyBox;
    std::vector<TextData> texts;
    glm::vec3 bgColor;

private:
    // Load-functions for each type
    void loadModelToScene(JSONModel model);
    void loadUnitPlaneToScene(JSONUnitPlane unitPlane);
    void loadGridToScene(JSONGrid grid);
    void loadSkyBoxToScene(JSONSkybox skyBox);
    void loadTextToScene(JSONText text);
};

#endif