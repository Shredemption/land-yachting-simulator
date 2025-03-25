#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <queue>
#include <mutex>

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

struct PendingTexture {
    std::string name;
    int width, height, channels;
    std::vector<unsigned char> pixelData;
    std::string typeName;
    unsigned int textureID = 0;
};

class Model
{
public:
    Model(std::tuple<std::string, std::string, std::string> NamePathShader);
    ~Model();

    void uploadToGPU();

    std::map<std::string, Bone *> boneHierarchy;
    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> boneOffsets;
    std::vector<glm::mat4> boneInverseOffsets;
    std::vector<Bone *> rootBones;
    std::string path;
    std::string name;

    static std::map<std::string, std::pair<std::string, ModelType>> modelMap;
    static void loadModelMap();

    std::vector<Mesh> meshes;
    std::string directory;

    static std::unordered_map<std::string, CachedTexture> textureCache;
    static std::mutex textureCacheMutex;
    static std::queue<PendingTexture> textureQueue;
    static std::mutex textureQueueMutex;
    static std::unordered_set<std::string> pendingTextures;
    static std::mutex pendingTexturesMutex;
    static std::mutex openglMutex;

    static unsigned int TextureFromFile(const char *name, const std::string &directory);
    static GLuint CreatePlaceholderTexture();
    void processPendingTextures();
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