#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <unordered_set>

enum class shaderID;
enum class ModelType;
class Model;

struct Texture
{
    unsigned int index;
    std::string path;

    bool operator==(const Texture &other) const
    {
        return this->path == other.path; // Compare based on path or other identifiers
    }

    int textureUnit = -1;
};

struct PendingTexture
{
    std::string path;
    int width, height, channels;
    std::vector<unsigned char> pixelData;
    unsigned int textureID = -1;
    int textureUnit = -1;
    bool repeating;
};

inline bool ends_with(std::string const &value, std::string const &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

struct TextureArray
{
    GLuint textureArrayID = 0;
    int width = 0;
    int height = 0;
    std::vector<PendingTexture> pendingTextures;
    std::unordered_map<std::string, int> textureLayerMap;

    int textureUnit = -1;
};

struct SkyBoxData;

class TextureManager
{
public:
    // Texture Arrays
    static std::unordered_map<std::string, TextureArray> textureArrays;
    static std::mutex textureArrayMutex;
    static std::unordered_map<std::string, Texture> standaloneTextureCache;
    static std::mutex standaloneCacheMutex;

    // Pending Textures
    static std::queue<PendingTexture> textureQueue;
    static std::mutex textureQueueMutex;
    static std::unordered_set<std::string> pendingTextures;
    static std::mutex pendingTexturesMutex;
    static std::mutex openglMutex;

    static void loadTexturesForShader(const shaderID &shader, const std::string &directory, ModelType &modelType, std::vector<std::string> &outTexturePaths, std::string &outTextureArrayName);

    static unsigned int loadStandaloneTexture(const std::string &filepath);

    static void queueStandaloneTexture(const std::string &fileName);
    static void queueStandaloneImage(const std::string &fileName);
    static void queueTextureToArray(const std::string &arrayName, const std::string &texturePath);
    static void queueTextureToArrayByFilename(const std::string &fileName, const std::string &arrayName);
    static unsigned int loadSkyboxTexture(const SkyBoxData &skybox);

    static void clearTextures();
    static void uploadToGPU();
    static void loadQueuedPixelData();

    static std::string getTextureArrayName(ModelType modelType);
    static GLuint getStandaloneTextureID(const std::string &texturePath);
    static GLuint getTextureArrayID(const std::string &arrayName);
    static GLuint getStandaloneTextureUnit(const std::string &texturePath);
    static GLuint getTextureArrayUnit(const std::string &arrayName);
    static int getTextureLayerIndex(const std::string &arrayName, const std::string &texturePath);
    static void getTextureData(const Model &model, unsigned int &textureUnit, unsigned int &textureArrayID, std::vector<int> &textureLayers);

private:
    static void uploadStandalones();
    static void uploadTextureArrays();

    static void clearStandaloneCache();
    static void clearTextureArrays();

    static void queueStandalone(const std::string &path, bool repeating);

    static std::vector<std::string> loadMaterialTexturePaths(const std::string &type, const std::string &directory);
};

#endif