#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <optional>

#include "mesh/mesh_util.hpp"
#include "mesh/meshvariant.h"
#include "physics/physicsbuffer.h"
#include "shader/shaderID.h"

class Model;

struct JSONModel
{
    std::string name = "none";
    std::vector<float> scale = {1, 1, 1};
    float angle = 0;
    std::vector<float> rotationAxis = {0, 1, 0};
    std::vector<float> translation = {0, 0, 0};
    std::string shader = "default";
    std::vector<float> color = {1, 0, 0};
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
    std::string shader = "flat";
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

struct JSONImage
{
    std::string file = "";
    std::vector<int> size = {0, 0};
    std::vector<float> position = {0.5, 0.5};
    std::vector<float> scale = {1, 1};
    float alpha = 1.0f;
    float rotation = 0.0f;
    bool mirrored = false;
};

struct JSONScene
{
    std::vector<JSONModel> models = {};
    std::vector<JSONUnitPlane> unitPlanes = {};
    std::vector<JSONGrid> grids = {};
    std::vector<JSONSkybox> skyBox = {};
    std::vector<JSONText> texts = {};
    std::vector<JSONImage> images = {};
    std::vector<float> bgColor = {0, 0, 0};
    std::vector<float> cameraPos = {0, 0, 0};
    std::vector<float> cameraDir = {0, 0, 0};
};

struct ModelData
{
    Model *model;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    shaderID shader;
    glm::vec3 color;
    bool animated;
    bool controlled;
    std::optional<PhysicsBuffer> physics;
};

struct UnitPlaneData
{
    glm::vec3 color;
    glm::mat4 u_model;
    glm::mat3 u_normal;
    shaderID shader;
    MeshVariant unitPlane = MeshUtil::genUnitPlane<VertexTextured>(color, shader);

    // Data from transparent rendering
    glm::vec3 position;
    bool isTransparent() const
    {
        // Add logic to check whether the shader is transparent or not
        return (shader == shaderID::Water);
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
    MeshVariant grid = MeshUtil::genGrid<VertexTextured>(gridSize.x, gridSize.y, lod, color, shader);
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

struct ImageData
{
    std::string file;
    int width, height;
    glm::vec2 position;
    glm::vec2 scale;
    float alpha;
    float rotation;
    bool mirrored;
    unsigned int textureID;
};