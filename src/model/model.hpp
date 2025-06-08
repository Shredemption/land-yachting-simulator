#pragma once

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <vector>
#include <map>

#include "mesh/meshvariant.h"
#include "model/model_defs.h"

enum class shaderID;
struct Bone;

class Model
{
public:
    Model(std::tuple<std::string, std::vector<std::string>, shaderID, ModelType> name_paths_shader_type);
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

    std::vector<std::string> paths;
    std::string name;
    ModelType modelType;

    std::vector<std::string> texturePaths;
    std::string textureArrayName;

    std::vector<std::vector<MeshVariant>> lodMeshes;
    std::string directory;

    // Generate and update bones
    void generateBoneTransforms();
    void generateBoneTransformsRecursive(Bone *bone);
    void updateBoneTransforms(std::vector<glm::mat4> &targetBones);
    void updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset, std::vector<glm::mat4> &targetBones);

    // Draw meshes in model
    void draw(int lodIndex);

private:
    void loadModel(const std::vector<std::string> &lodPaths, shaderID &shader);
    void processNode(aiNode *node, const aiScene *scene, shaderID &shader, std::vector<MeshVariant> &targetMeshList, Bone *parentBone = nullptr);
    MeshVariant processMesh(aiMesh *mesh, const aiScene *scene, shaderID &shader, std::map<std::string, Bone *> &boneHierarchy);

    template <typename VertexType>
    Mesh<VertexType> combineMeshes(const std::vector<Mesh<VertexType>> &meshes);
    MeshVariant combineMeshVariants(const std::vector<MeshVariant> &variants);
};