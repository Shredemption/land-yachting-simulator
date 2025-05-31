#include "texture_manager/texture_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>

#include "scene/scene.h"

// Texture Cache and Array
std::unordered_map<std::string, CachedTexture> TextureManager::textureCache;
std::mutex TextureManager::textureCacheMutex;
GLuint TextureManager::textureArrayID;
std::unordered_map<std::string, int> TextureManager::textureLayerMap;

// Texture Queue of to be loaded textures
std::queue<PendingTexture> TextureManager::textureQueue;
std::mutex TextureManager::textureQueueMutex;

// Names of textures that are to be loaded
std::unordered_set<std::string> TextureManager::pendingTextures;
std::mutex TextureManager::pendingTexturesMutex;
std::mutex TextureManager::openglMutex;

void TextureManager::loadTexturesForShader(const aiMesh* mesh, const aiScene* scene, const shaderID& shader, const std::string& directory, std::vector<Texture>& textures)
{
    // Process Mats
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Load textures based on shader
        if (shader == shaderID::shDefault)
        {
            // Check and add diffuse maps if not already loaded
            std::vector<Texture> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "diffuse", directory);
            for (const auto &texture : diffuseMaps)
            {
                if (std::find(textures.begin(), textures.end(), texture) == textures.end()) // Texture not found
                {
                    textures.push_back(texture);
                }
            }

            // Check and add normal maps if not already loaded
            std::vector<Texture> normalMaps = loadMaterialTexture(material, aiTextureType_UNKNOWN, "properties", directory);
            for (const auto &texture : normalMaps)
            {
                if (std::find(textures.begin(), textures.end(), texture) == textures.end()) // Texture not found
                {
                    textures.push_back(texture);
                }
            }
        }

        if (shader == shaderID::shToon)
        {
            // Check and add toon textures (e.g., highlight and shadow) if not already loaded
            std::vector<Texture> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "highlight", directory);
            for (const auto &texture : diffuseMaps)
            {
                if (std::find(textures.begin(), textures.end(), texture) == textures.end()) // Texture not found
                {
                    textures.push_back(texture);
                }
            }

            std::vector<Texture> normalMaps = loadMaterialTexture(material, aiTextureType_UNKNOWN, "shadow", directory);
            for (const auto &texture : normalMaps)
            {
                if (std::find(textures.begin(), textures.end(), texture) == textures.end()) // Texture not found
                {
                    textures.push_back(texture);
                }
            }
        }
    }
}

std::vector<Texture> TextureManager::loadMaterialTexture(const aiMaterial *mat, const aiTextureType type, const std::string typeName, const std::string directory)
{
    // Vector of textures to push to GPU
    std::vector<Texture> loadTextures;
    std::string textureName = findTextureInDirectory(directory, typeName);

    // If texture found
    if (!textureName.empty())
    {
        {
            // Check if texture already pending
            std::lock_guard<std::mutex> lock(pendingTexturesMutex);
            if (pendingTextures.find(textureName) != pendingTextures.end())
            {
                // Another thread is already loading this texture
                return loadTextures;
            }

            // If not, add to pending
            pendingTextures.insert(textureName);
        }

        // Placeholder texture for model loading
        Texture placeholderTexture;
        placeholderTexture.index = 0;
        placeholderTexture.type = typeName;
        placeholderTexture.path = textureName;
        loadTextures.push_back(placeholderTexture);

        // Get texture location
        std::string filename = directory + '/' + textureName;
        int width, height, channels;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            {
                std::lock_guard<std::mutex> lock(pendingTexturesMutex);
                pendingTextures.erase(textureName);
            }
            return loadTextures;
        }

        // Save PendingTexture
        PendingTexture texture;
        texture.name = textureName;
        texture.width = width;
        texture.height = height;
        texture.channels = channels;
        texture.pixelData.assign(data, data + (width * height * channels));
        texture.typeName = typeName;
        texture.textureID = 0; // Placeholder ID

        // Unload data
        stbi_image_free(data);

        {
            // Push texture to queue
            std::lock_guard<std::mutex> lock(textureQueueMutex);
            textureQueue.push(std::move(texture));
        }
    }

    // If texture not found
    else
    {
        std::cout << "Failed to load " << typeName << " texture in " << directory << "\n";
    }

    return loadTextures;
}

void TextureManager::processPendingTextures()
{
    std::lock_guard<std::mutex> lock(textureQueueMutex);
    std::vector<PendingTexture> textures;

    while (!textureQueue.empty())
    {
        textures.push_back(textureQueue.front());
        textureQueue.pop();
    }

    if (textures.empty())
        return;

    int texWidth = textures[0].width;
    int texHeight = textures[0].height;
    int texChannels = textures[0].channels;

    for (const auto &tex : textures)
    {
        if (tex.width != texWidth || tex.height != texHeight || tex.channels != texChannels)
        {
            std::cerr << "Inconsistent texture dimensions or formats!\n";
            return;
        }
    }

    glGenTextures(1, &textureArrayID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    GLenum internalFormat = (texChannels == 4) ? GL_RGBA8 : GL_RGB8;
    GLenum format = (texChannels == 4) ? GL_RGBA : GL_RGB;

    // Allocate 3D texture storage
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, texWidth, texHeight, static_cast<GLsizei>(textures.size()), 0, format, GL_UNSIGNED_BYTE, nullptr);

    // Upload each layer
    for (size_t i = 0; i < textures.size(); ++i)
    {
        const auto &tex = textures[i];
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                        0, 0, i,
                        texWidth, texHeight, 1,
                        format, GL_UNSIGNED_BYTE,
                        tex.pixelData.data());
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Texture layer map
    for (size_t i = 0; i < textures.size(); ++i)
    {
        textureLayerMap[textures[i].name] = static_cast<int>(i);
    }

    // Activate and Bind texture array
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
}

void TextureManager::unloadTextures()
{
    glDeleteTextures(1, &textureArrayID);
    textureArrayID = 0;

    textureCache.clear();
    pendingTextures.clear();
}

std::string TextureManager::findTextureInDirectory(const std::string &directory, const std::string &typeName)
{
    // Define common texture file extensions
    const std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga"};

    // Iterate through files in the directory
    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        std::string filename = entry.path().filename().string();

        // Check if the filename contains the texture type (e.g., "normal", "roughness", etc.)
        if (filename.find(typeName) != std::string::npos)
        {
            // Check if the file has a valid texture extension
            for (const auto &ext : extensions)
            {
                if (ends_with(filename, ext))
                {
                    return filename; // Return the first matching texture
                }
            }
        }
    }

    return ""; // Return empty if no matching texture is found
}

unsigned int TextureManager::TextureFromFile(const char *name, const std::string &directory)
{
    // Get texture location
    std::string filename = std::string(name);
    filename = directory + '/' + filename;

    // Generate empty texture
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // Load texture file
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        // Detect what kind of texture
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        // Bind texture and upload
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture looping and scaling parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Unload texture
        stbi_image_free(data);
    }
    else
    {
        // Error if texture cant be loaded
        std::cout << "Texture failed to load at path: " << name << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

Texture TextureManager::LoadStandaloneTexture(std::string fileName)
{
    Texture loadTexture;
    // If texture already loaded
    if (TextureManager::textureCache.find(fileName) != TextureManager::textureCache.end())
    {
        // Use cached texture
        loadTexture = TextureManager::textureCache[fileName].texture;
        TextureManager::textureCache[fileName].refCount++;
    }
    else
    {
        // Define and load new texture to texture cache
        Texture texture;
        texture.index = TextureManager::TextureFromFile(fileName.c_str(), "../resources/textures");
        texture.type = "standalone";
        texture.path = fileName.c_str();

        loadTexture = texture;
        TextureManager::textureCache[fileName].texture = texture;
    }

    return loadTexture;
}

Texture TextureManager::LoadImageToTexture(std::string fileName)
{
    Texture loadTexture;
    // If texture already loaded
    if (TextureManager::textureCache.find(fileName) != TextureManager::textureCache.end())
    {
        // Use cached texture
        loadTexture = TextureManager::textureCache[fileName].texture;
        TextureManager::textureCache[fileName].refCount++;
    }
    else
    {
        // Define and load new texture to texture cache
        Texture texture;
        texture.index = TextureManager::TextureFromFile(fileName.c_str(), "../resources/images");
        texture.type = "image";
        texture.path = fileName.c_str();

        loadTexture = texture;
        TextureManager::textureCache[fileName].texture = texture;
    }

    return loadTexture;
}

unsigned int TextureManager::LoadSkyBoxTexture(const SkyBoxData &skybox)
{
    std::vector<Texture> loadTextures;

    std::vector<std::string> SkyBoxTextureNames =
        {
            skybox.right,
            skybox.left,
            skybox.up,
            skybox.down,
            skybox.front,
            skybox.back,
        };

    // Gen and bind cubemap texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // Load texture
    int width, height, nrChannels;
    unsigned char *data;
    for (unsigned int i = 0; i < 6; i++)
    {
        data = stbi_load(SkyBoxTextureNames[i].c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            // Unload texture
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << SkyBoxTextureNames[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // Set texture parameters

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void TextureManager::uploadToGPU()
{
    // Process all pending textures of model
    processPendingTextures();
}