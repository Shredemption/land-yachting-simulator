#ifndef MODEL_H
#define MODEL_H

#include <mesh/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Model
{
public:
    unordered_map<string, Texture> textureCache;

    // Model Constructor
    Model(string const &path);

    // Model Renderer
    void Draw(Shader &shader);

private:
    // Model data
    vector<Mesh> meshes;
    string directory;

    // Model Loading
    void loadModel(string path);

    // Node Processor
    void processNode(aiNode *node, const aiScene *scene);

    // Mesh Processor
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        
    // Material Texture Loader
    vector<Texture> loadMaterialTexture(aiMaterial *mat, aiTextureType type, string typeName);
};

#endif // MODEL_H