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
#include <variant>

#include "mesh/mesh.h"

struct Texture
{
    unsigned int index;
    std::string type;
    std::string path;

    bool operator==(const Texture &other) const
    {
        return this->path == other.path; // Compare based on path or other identifiers
    }
};

// Keep count of how many times cached texture is used
struct CachedTexture
{
    Texture texture;
    int refCount;
};

// Empty struct for json unwrapping
struct SkyBoxData;

struct JSONModelMapEntry
{
    std::string mainPath;
    std::vector<std::string> lodPaths = {};
    std::string type = "model";
};

struct JSONModelMap
{
    std::map<std::string, JSONModelMapEntry> yachts;
    std::map<std::string, JSONModelMapEntry> models;
};

struct PendingTexture
{
    std::string name;
    int width, height, channels;
    std::vector<unsigned char> pixelData;
    std::string typeName;
    unsigned int textureID = 0;
};

using MeshVariant = std::variant<
    Mesh<VertexAnimated>,
    Mesh<VertexSimple>,
    Mesh<VertexTextured>>;

class Model
{
public:
    Model(std::tuple<std::string, std::vector<std::string>, shaderID> name_paths_shader);
    ~Model();

    void uploadToGPU();

    // Local model data
    std::map<std::string, Bone *> boneHierarchy;
    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> boneOffsets;
    std::vector<glm::mat4> boneInverseOffsets;
    std::vector<Bone *> rootBones;
    std::vector<std::string> paths;
    std::string name;
    std::vector<Texture> textures;

    // Model map and load function
    static std::map<std::string, JSONModelMapEntry> modelMap;
    static void loadModelMap();

    std::vector<std::vector<MeshVariant>> lodMeshes;
    float distanceFromCamera;
    std::string directory;

    // Texture cache and Array
    static std::unordered_map<std::string, CachedTexture> textureCache;
    static std::mutex textureCacheMutex;
    static GLuint textureArrayID;
    static std::unordered_map<std::string, int> textureLayerMap;

    // Pending Textures
    static std::queue<PendingTexture> textureQueue;
    static std::mutex textureQueueMutex;
    static std::unordered_set<std::string> pendingTextures;
    static std::mutex pendingTexturesMutex;
    static std::mutex openglMutex;

    // Load textures
    static unsigned int TextureFromFile(const char *name, const std::string &directory);
    void processPendingTextures();
    static unsigned int LoadSkyBoxTexture(SkyBoxData skybox);
    static void unloadTextures();

    // Generate and update bones
    void generateBoneTransforms();
    void generateBoneTransformsRecursive(Bone *bone);
    void updateBoneTransforms();
    void updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset);

    // Draw meshes in model
    void draw(int lodIndex);

private:
    void loadModel(const std::vector<std::string> &lodPaths, shaderID &shader);
    void processNode(aiNode *node, const aiScene *scene, shaderID &shader, std::vector<MeshVariant> &targetMeshList, Bone *parentBone = nullptr);
    MeshVariant processMesh(aiMesh *mesh, const aiScene *scene, shaderID &shader, std::map<std::string, Bone *> &boneHierarchy);
    // Mesh combineMeshes(const std::vector<Mesh> &meshes);
    void loadTexturesForShader(aiMesh *mesh, const aiScene *scene, const shaderID &shader);
    std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName);
    std::string findTextureInDirectory(const std::string &directory, const std::string &typeName);

    inline bool ends_with(std::string const &value, std::string const &ending);
};

#endif