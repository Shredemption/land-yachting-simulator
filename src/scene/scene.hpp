#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include "scene/scene_defs.h"

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
    std::vector<ImageData> images;
    glm::vec3 bgColor;
    glm::vec3 cameraPos;
    glm::vec3 cameraDir;

    // Global Light properties
    glm::vec3 lightPos = {1000.0f, -1000.0f, 2000.0f};
    glm::vec3 lightCol = {1.0f, 1.0f, 1.0f};
    float lightInsensity = 2;

private:
    // Load-functions for each type
    void loadModelToScene(JSONModel model);
    void loadUnitPlaneToScene(JSONUnitPlane unitPlane);
    void loadGridToScene(JSONGrid grid);
    void loadSkyBoxToScene(JSONSkybox skyBox);
    void loadTextToScene(JSONText text);
    void loadImageToScene(JSONImage image);
};