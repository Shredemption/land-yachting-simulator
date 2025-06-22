#pragma once

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <vector>
#include <optional>
#include <map>

#include "mesh/meshvariant.h"
#include "model/model_defs.h"

enum class shaderID;
struct Bone;
struct LoadModelData;

class Model
{
public:
    Model(LoadModelData &loadModelData);
    ~Model();

    void uploadToGPU();

    // Local model data
    std::map<std::string, Bone *> boneHierarchy;
    std::vector<glm::mat4> boneOffsets;
    std::vector<glm::mat4> boneInverseOffsets;
    std::vector<Bone *> rootBones;

    std::vector<glm::mat4> boneTransforms[2];
    const std::vector<glm::mat4> &getReadBuffer();
    std::vector<glm::mat4> &getWriteBuffer();

    std::string name;
    ModelType modelType;

    std::vector<std::string> texturePaths;
    std::string textureArrayName;

    std::vector<std::vector<MeshVariant>> lodMeshes;
    std::optional<std::vector<MeshVariant>> hitboxMeshes;
    std::string directory;

    // Generate and update bones
    void generateBoneTransforms();
    void generateBoneTransformsRecursive(Bone *bone);
    void updateBoneTransforms(std::vector<glm::mat4> &targetBones);
    void updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset, std::vector<glm::mat4> &targetBones);

    // Draw meshes in model
    void draw(int lodIndex);

private:
    void loadModel(std::string mainPath, std::optional<std::vector<std::string>> lodPaths, std::optional<std::string> hitboxPath, bool hasPhysics, shaderID &shader);
    void processNode(aiNode *node, const aiScene *scene, shaderID &shader, std::vector<MeshVariant> &targetMeshList, Bone *parentBone = nullptr);
    MeshVariant processMesh(aiMesh *mesh, const aiScene *scene, shaderID &shader, std::map<std::string, Bone *> &boneHierarchy);
    void loadHitbox(std::optional<std::string> hitboxPath);
    void processHitboxNode(aiNode *node, const aiScene *scene, std::vector<MeshVariant> &hitboxMeshes);

    template <typename VertexType>
    Mesh<VertexType> combineMeshes(const std::vector<Mesh<VertexType>> &meshes);
    MeshVariant combineMeshVariants(const std::vector<MeshVariant> &variants);
};