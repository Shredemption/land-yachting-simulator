#ifndef MODEL_H
#define MODEL_H

#include <mesh/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <map>
#include <vector>

// Keep count of how many times cached texture is used
struct CachedTexture
{
    Texture texture;
    int refCount;
};

struct SkyBoxData;

class Model
{
public:
    // Model Constructor
    Model(std::pair<std::string, std::string> pathShader);

    std::map<std::string, Bone *> boneHierarchy;
    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> boneOffsets;
    std::vector<glm::mat4> boneInverseOffsets;
    std::vector<Bone *> rootBones;
    std::string path;

    // Model Destructor
    ~Model();

    // Load JSON model map
    static std::map<std::string, std::string> modelMap;
    static std::map<std::string, std::string> loadModelMap(const std::string &filePath);

    // Model data
    std::vector<Mesh> meshes;
    std::string directory;

    // Total texture cache
    static std::unordered_map<std::string, CachedTexture> textureCache;

    // Texture from File
    static unsigned int TextureFromFile(const char *name, const std::string &directory);

    static unsigned int LoadSkyBoxTexture(SkyBoxData skybox);

    void generateBoneTransforms();
    void generateBoneTransformsRecursive(Bone *bone);
    void updateBoneTransforms();
    void updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset);

private:
    // Model Loading
    void loadModel(std::string path, std::string shaderName);

    // Node Processor
    void processNode(aiNode *node, const aiScene *scene, std::string shaderName, Bone *parentBone = nullptr);

    // Mesh Processor
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, std::string shaderName, std::map<std::string, Bone *> &boneHierarchy);

    // Material Texture Loader
    std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName);

    // Find textures in directory
    std::string findTextureInDirectory(const std::string &directory, const std::string &typeName);

    // Check if string ends with x
    inline bool ends_with(std::string const &value, std::string const &ending);
};

#endif // MODEL_H