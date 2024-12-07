#include <model/model.h>

#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int TextureFromFile(const char *path, const std::string &directory);

std::unordered_map<std::string, CachedTexture> Model::textureCache;

// Model Constructor
Model::Model(std::string const &path)
{
    loadModel(path);
}

// Model Destructor
Model::~Model()
{
    // For every mesh
    for (const auto &mesh : meshes)
    {
        // For every texture in mesh
        for (const auto &texture : mesh.textures)
        {
            // Check if texture still in textureCache
            auto iteration = textureCache.find(texture.path);
            if (iteration != textureCache.end())
            {
                // If texture in cache, decrement count
                iteration->second.refCount--;

                // If count 0
                if (iteration->second.refCount == 0)
                {
                    // Remove unload from GPU and remove from cache
                    glDeleteTextures(1, &iteration->second.texture.id);
                    textureCache.erase(iteration);
                }
            }
        }
    }

    // Release mesh VAO, VBO and EBO from GPU
    glDeleteBuffers(1, &meshes[0].VBO);
    glDeleteBuffers(1, &meshes[0].EBO);
    glDeleteVertexArrays(1, &meshes[0].VAO);

    // Ensure meshes vector clears properly
    meshes.clear();
}

// Model Renderer
void Model::Draw(Shader &shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(std::string path)
{
    // Define importer and open file
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    // If scene null, scene flagged as incomplete, or root node null
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        // Show error
        std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Open full node recursion
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // Process node's meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // Repeat for children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // Process vertex positions
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // Process normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        // Process texture coords
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // Process Indx
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // Process Mats
    if (mesh->mMaterialIndex >= 0)
    {
        // Get material
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Instert diffuse and specular maps to textures
        std::vector<Texture> diffuseMaps = loadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    // Vector of textures to push to GPU
    std::vector<Texture> textures;

    // For every texture
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;

        // Find texture
        mat->GetTexture(type, i, &str);
        std::string texturePath = str.C_Str();

        // If texture already loaded loaded
        if (textureCache.find(texturePath) != textureCache.end())
        {
            // Use cached texture
            textures.push_back(textureCache[texturePath].texture);
            textureCache[texturePath].refCount++;
        }
        else
        {
            // Define and load new texture to texture cache
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textureCache[texturePath].texture = texture;
        }
    }
    return textures;
}

unsigned int TextureFromFile(const char *path, const std::string &directory)
{
    // Get texture location
    std::string filename = std::string(path);
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
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
};