#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <queue>

#include "shader/shader.h"

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

struct PendingTexture
{
    std::string name;
    int width, height, channels;
    std::vector<unsigned char> pixelData;
    std::string typeName;
    unsigned int textureID = 0;
};

inline bool ends_with(std::string const &value, std::string const &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

struct SkyBoxData;

class TextureManager
{
public:
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
    static void processPendingTextures();
    static unsigned int LoadSkyBoxTexture(const SkyBoxData &skybox);
    static void unloadTextures();

    static void loadTexturesForShader(const aiMesh* mesh, const aiScene* scene, const shaderID& shader, const std::string& directory, std::vector<Texture>& textures);
    static std::vector<Texture> loadMaterialTexture(const aiMaterial *mat, const aiTextureType type, const std::string typeName, const std::string directory);
    static std::string findTextureInDirectory(const std::string &directory, const std::string &typeName);

    static Texture LoadStandaloneTexture(std::string fileName);
    static Texture LoadImageToTexture(std::string fileName);

    static void uploadToGPU();
};

#endif