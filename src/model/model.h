#ifndef MODEL_H
#define MODEL_H

#include <mesh/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <vector>

class Model
{
public:
    std::unordered_map<std::string, Texture> textureCache;

    // Model Constructor
    Model(std::string const &path);

    // Model Renderer
    void Draw(Shader &shader);

private:
    // Model data
    std::vector<Mesh> meshes;
    std::string directory;

    // Model Loading
    void loadModel(std::string path);

    // Node Processor
    void processNode(aiNode *node, const aiScene *scene);

    // Mesh Processor
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        
    // Material Texture Loader
    std::vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName);
};

#endif // MODEL_H