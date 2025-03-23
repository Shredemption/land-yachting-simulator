#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <map>
#include <vector>

#include "mesh/mesh.h"

// Keep count of how many times cached texture is used
struct CachedTexture
{
    Texture texture;
    int refCount;
};

struct SkyBoxData;

enum ModelType
{
    model,
    yacht
};

struct JSONModelMapData
{
    std::string name;   
    std::string path;
};

struct JSONModelMap
{
    std::vector<JSONModelMapData> yachts;
    std::vector<JSONModelMapData> models;
};

class Model
{
public:
    Model(std::pair<std::string, std::string> pathShader);
    ~Model();

    std::map<std::string, Bone *> boneHierarchy;
    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> boneOffsets;
    std::vector<glm::mat4> boneInverseOffsets;
    std::vector<Bone *> rootBones;
    std::string path;

    static std::map<std::string, std::pair<std::string, ModelType>> modelMap;
    static void loadModelMap();

    std::vector<Mesh> meshes;
    std::string directory;

    static std::unordered_map<std::string, CachedTexture> textureCache;

    static unsigned int TextureFromFile(const char *name, const std::string &directory);
    static unsigned int LoadSkyBoxTexture(SkyBoxData skybox);

    void generateBoneTransforms();
    void generateBoneTransformsRecursive(Bone *bone);
    void updateBoneTransforms();
    void updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset);

private:
    void loadModel(std::string path, std::string shaderName);
    void processNode(aiNode *node, const aiScene *scene, std::string shaderName, Bone *parentBone = nullptr);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string shaderName, std::map<std::string, Bone *> &boneHierarchy);
    std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName);
    std::string findTextureInDirectory(const std::string &directory, const std::string &typeName);

    inline bool ends_with(std::string const &value, std::string const &ending);
};

#endif // MODEL_H