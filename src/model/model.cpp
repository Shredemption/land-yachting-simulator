#include "model/model.hpp"

#include "pch.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

template <typename MeshType>
struct ExtractVertexType;

template <typename VertexType>
struct ExtractVertexType<Mesh<VertexType>>
{
    using type = VertexType;
};

// Model Constructor
Model::Model(std::tuple<std::string, std::vector<std::string>, shaderID, ModelType> name_paths_shader_type)
{
    this->name = std::get<0>(name_paths_shader_type);
    this->paths = std::get<1>(name_paths_shader_type);
    this->modelType = std::get<3>(name_paths_shader_type);

    loadModel(paths, std::get<2>(name_paths_shader_type));
}

// Model Destructor
Model::~Model()
{
    for (auto &meshes : lodMeshes)
    {
        for (auto &meshVariant : meshes)
        {
            std::visit([](auto &mesh)
                       {
                // Release mesh VAO, VBO and EBO from GPU
                glDeleteBuffers(1, &mesh.VBO);
                glDeleteBuffers(1, &mesh.EBO);
                glDeleteVertexArrays(1, &mesh.VAO); }, meshVariant);
        }
    }

    // Ensure meshes vector clears properly
    lodMeshes.clear();
}

void Model::loadModel(const std::vector<std::string> &lodPaths, shaderID &shader)
{
    for (size_t i = 0; i < lodPaths.size(); i++)
    {
        // Define importer and open file
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(lodPaths[i], aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);

        // If scene null, scene flagged as incomplete, or root node null
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "Assimp Error (" << lodPaths[i] << "): " << importer.GetErrorString() << std::endl;
            continue;
        }

        // Open full node recursion
        directory = lodPaths[i].substr(0, lodPaths[i].find_last_of('/'));

        std::vector<MeshVariant> lodLevelMeshes;

        processNode(scene->mRootNode, scene, shader, lodLevelMeshes, nullptr);

        MeshVariant combinedMesh = combineMeshVariants(lodLevelMeshes);
        lodMeshes.push_back({std::move(combinedMesh)});
    }

    TextureManager::loadTexturesForShader(shader, directory, modelType, texturePaths, textureArrayName);

    // Generate initial bone positions
    generateBoneTransforms();
}

void Model::processNode(aiNode *node, const aiScene *scene, shaderID &shader, std::vector<MeshVariant> &targetMeshList, Bone *parentBone)
{
    // Get node name
    std::string nodeName = node->mName.C_Str();

    // If node is armatrue, extract bone
    if (nodeName.rfind("Armature", 0) == 0)
    {
        if (boneHierarchy.find(nodeName) == boneHierarchy.end())
        {
            Bone *currentBone = new Bone(nodeName, boneHierarchy.size() - 1, glm::mat4(1.0f));
            boneHierarchy.emplace(nodeName, currentBone);

            if (parentBone)
            {
                parentBone->children.push_back(currentBone);
                currentBone->parent = parentBone;
            }
            else
            {
                rootBones.push_back(currentBone);
            }
        }
    }

    // Else extract meshes
    else
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            unsigned int meshIndex = node->mMeshes[i];
            aiMesh *mesh = scene->mMeshes[meshIndex];
            targetMeshList.push_back(processMesh(mesh, scene, shader, boneHierarchy));
        }
    }

    // Recursively process children
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene, shader, targetMeshList, boneHierarchy[nodeName]);
    }
}

MeshVariant Model::processMesh(aiMesh *mesh, const aiScene *scene, shaderID &shader, std::map<std::string, Bone *> &boneHierarchy)
{
    std::vector<unsigned int> indices;

    bool isAnimated = (shader == shaderID::Default || shader == shaderID::Toon);

    // Process Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }

    if (isAnimated)
    {
        std::vector<VertexAnimated> vertices(mesh->mNumVertices);

        // Pull vertex data
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexAnimated &vertex = vertices[i];
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            // If texture present
            if (mesh->mTextureCoords[0])
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.TexCoords = glm::vec2(0.0f);
        }

        // Process Bone ID's
        for (unsigned int i = 0; i < mesh->mNumBones; i++)
        {
            aiBone *bone = mesh->mBones[i];
            std::string boneName = bone->mName.C_Str();
            glm::mat4 offsetMatrix = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
            boneHierarchy[boneName]->offsetMatrix = glm::inverse(offsetMatrix);
            int boneIndex = boneHierarchy[boneName]->index;

            // Process weights per bone
            for (unsigned int j = 0; j < bone->mNumWeights; j++)
            {
                int vertexID = bone->mWeights[j].mVertexId;
                float weightValue = bone->mWeights[j].mWeight;

                // Add this bone's influence to the vertex
                for (int k = 0; k < 4; k++)
                {
                    if (vertices[vertexID].Weights[k] == 0.0f)
                    { // Find an empty slot
                        vertices[vertexID].BoneIDs[k] = boneIndex;
                        vertices[vertexID].Weights[k] = weightValue;
                        break;
                    }
                }
            }
        }

        // Remove "Scene" bone if it is present
        auto it = boneHierarchy.find("Scene");
        if (it != boneHierarchy.end())
        {
            boneHierarchy.erase(it); // Remove the "Scene" bone from the hierarchy
        }

        return Mesh<VertexAnimated>(vertices, indices, shader);
    }

    else
    {
        std::vector<VertexTextured> vertices(mesh->mNumVertices);

        // Pull vertex data
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexTextured &vertex = vertices[i];
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            // If texture present
            if (mesh->mTextureCoords[0])
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                vertex.TexCoords = glm::vec2(0.0f);
        }

        return Mesh<VertexTextured>(vertices, indices, shader);
    }
}

template <typename VertexType>
Mesh<VertexType> Model::combineMeshes(const std::vector<Mesh<VertexType>> &meshes)
{
    std::vector<VertexType> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int indexOffset = 0;

    shaderID shader = meshes[0].shader;

    for (const auto &mesh : meshes)
    {
        allVertices.insert(allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        for (unsigned int idx : mesh.indices)
        {
            allIndices.push_back(idx + indexOffset);
        }

        indexOffset += mesh.vertices.size();
    }

    return Mesh<VertexType>(allVertices, allIndices, shader);
}

MeshVariant Model::combineMeshVariants(const std::vector<MeshVariant> &variants)
{
    if (variants.empty())
        throw std::runtime_error("Cannot combine empty variant list");

    return std::visit([&](auto &&mesh) -> MeshVariant
                      {
        using MeshType = std::decay_t<decltype(mesh)>;         // Mesh<VertexAnimated>, etc.
        using VertexType = typename ExtractVertexType<MeshType>::type;

        std::vector<MeshType> typedMeshes;
        for (const auto& variant : variants) {
            if (std::holds_alternative<MeshType>(variant))
                typedMeshes.push_back(std::get<MeshType>(variant));
            else
                throw std::runtime_error("Inconsistent mesh types in variant list");
        }

        Mesh<VertexType> combined = combineMeshes<VertexType>(typedMeshes); // correct now
        return MeshVariant{std::move(combined)}; }, variants[0]);
}

const std::vector<glm::mat4> &Model::getReadBuffer()
{
    return boneTransforms[ModelUtil::activeBoneBuffer.load(std::memory_order_acquire)];
}

std::vector<glm::mat4> &Model::getWriteBuffer()
{
    return boneTransforms[1 - ModelUtil::activeBoneBuffer.load(std::memory_order_acquire)];
}

void Model::generateBoneTransforms()
{
    // Resize if matrix array not large enough
    for (int i = 0; i < 2; i++)
    {
        if (boneTransforms[i].size() != boneHierarchy.size())
        {
            boneTransforms[i].resize(boneHierarchy.size(), glm::mat4(1.0f));
        }
    }

    if (boneOffsets.size() != boneHierarchy.size())
        boneOffsets.resize(boneHierarchy.size(), glm::mat4(1.0f));
    if (boneInverseOffsets.size() != boneHierarchy.size())
        boneInverseOffsets.resize(boneHierarchy.size(), glm::mat4(1.0f));

    for (auto &rootBone : rootBones)
    {
        // Start recursion from the root bone, with identity matrix for the root's parent transform
        generateBoneTransformsRecursive(rootBone);
    }
}

void Model::generateBoneTransformsRecursive(Bone *bone)
{
    // Check if bone in range
    if (bone->index < 0 || bone->index >= boneHierarchy.size())
    {
        std::cerr << "Error: Bone index out of range: " << bone->index << ", with name: " << bone->name << std::endl;
        return;
    }

    boneInverseOffsets[bone->index] = glm::inverse(bone->offsetMatrix);

    // Recursively update the transforms of the child bones
    for (Bone *child : bone->children)
    {
        if (!child)
        {
            continue; // Prevents null pointer access
        }
        generateBoneTransformsRecursive(child);
    }
}

void Model::updateBoneTransforms(std::vector<glm::mat4> &targetBones)
{
    for (auto &rootBone : rootBones)
    {
        // Start recursion from the root bone, with identity matrix for the root's parent transform
        updateBoneTransformsRecursive(rootBone, glm::mat4(1.0f), glm::mat4(1.0f), targetBones);
    }
}

void Model::updateBoneTransformsRecursive(Bone *bone, const glm::mat4 &parentTransform, const glm::mat4 &parentInverseOffset, std::vector<glm::mat4> &targetBones)
{
    // Check if bone in range
    if (bone->index < 0 || bone->index >= targetBones.size())
    {
        std::cerr << "Error: Bone index out of range: " << bone->index << ", with name: " << bone->name << std::endl;
        return;
    }

    // Update bone transform
    targetBones[bone->index] = parentTransform * parentInverseOffset * bone->offsetMatrix * bone->transform;

    // Recursively update the transforms of the child bones
    for (Bone *child : bone->children)
    {
        if (!child)
        {
            continue; // Prevents null pointer access
        }
        updateBoneTransformsRecursive(child, targetBones[bone->index], boneInverseOffsets[bone->index], targetBones);
    }
}

void Model::uploadToGPU()
{
    // Upload data for each mesh to GPU
    for (auto &meshes : lodMeshes)
    {
        for (auto &meshVariant : meshes)
        {
            std::visit([](auto &mesh)
                       { mesh.uploadToGPU(); },
                       meshVariant);
        }
    }
}

void Model::draw(int lodIndex)
{
    for (auto meshVariant : this->lodMeshes[lodIndex])
    {
        std::visit([](auto &mesh)
                   {
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0); },
                   meshVariant);
    }

    glBindVertexArray(0);
}