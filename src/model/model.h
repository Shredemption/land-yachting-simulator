#ifndef MODEL_H
#define MODEL_H

#include <mesh/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <vector>

// Keep count of how many times cached texture is used
struct CachedTexture {
    Texture texture;
    int refCount;
};

class Model
{
public:

    // Model Constructor
    Model(std::string const &path);

    // Model Destructor
    ~Model();

    // Model Renderer
    void Draw(Shader &shader);

private:
    // Model data
    std::vector<Mesh> meshes;
    std::string directory;

    // Total texture cache
    static std::unordered_map<std::string, CachedTexture> textureCache;

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