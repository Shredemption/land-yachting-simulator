#include "shader/shader_util.hpp"

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include "file_manager/file_manager.hpp"
#include "shader/shader.hpp"
#include "shader/shaderID.h"

Shader *ShaderUtil::load(const shaderID shaderID)
{
    Shader *shaderPtr;
    if (shaderID == lastShader)
    {
        shaderPtr = &loadedShaders[shaderID];
    }
    else
    {
        if (loadedShaders.find(shaderID) == loadedShaders.end())
        {
            Shader shader;
            shader.init(FileManager::read("resources/shaders/" + NameFromShader(shaderID) + ".vs"), FileManager::read("resources/shaders/" + NameFromShader(shaderID) + ".fs"));
            loadedShaders.emplace(shaderID, shader);
        }

        lastShader = shaderID;
        shaderPtr = &loadedShaders[shaderID];
        shaderPtr->use();
    }
    return shaderPtr;
}

void ShaderUtil::unload()
{
    // Clear global data from loading before loading new scene
    for (auto &shaderPair : loadedShaders)
    {
        glDeleteProgram(shaderPair.second.m_id); // Explicitly delete the shader program from the GPU
    }

    loadedShaders.clear();
    lastShader = shaderID::shNone;

    waterLoaded = false;
}

shaderID ShaderUtil::ShaderFromName(const std::string shaderName)
{
    static const std::unordered_map<std::string, shaderID> typeMap = {
        {"default", shaderID::shDefault},
        {"gui", shaderID::shGui},
        {"simple", shaderID::shSimple},
        {"text", shaderID::shText},
        {"image", shaderID::shImage},
        {"water", shaderID::shWater},
        {"skybox", shaderID::shSkybox},
        {"toon", shaderID::shToon},
        {"toon-terrain", shaderID::shToonTerrain},
        {"toon-water", shaderID::shToonWater},
        {"darken-blur", shaderID::shDarkenBlur},
        {"post", shaderID::shPost}};

    auto it = typeMap.find(shaderName);
    return it != typeMap.end() ? it->second : shaderID::shNone;
}

std::string ShaderUtil::NameFromShader(const shaderID shader)
{
    static const std::unordered_map<shaderID, std::string> typeMap = {
        {shaderID::shDefault, "default"},
        {shaderID::shGui, "gui"},
        {shaderID::shSimple, "simple"},
        {shaderID::shText, "text"},
        {shaderID::shImage, "image"},
        {shaderID::shWater, "water"},
        {shaderID::shSkybox, "skybox"},
        {shaderID::shToon, "toon"},
        {shaderID::shToonTerrain, "toon-terrain"},
        {shaderID::shToonWater, "toon-water"},
        {shaderID::shDarkenBlur, "darken-blur"},
        {shaderID::shPost, "post"}};

    auto it = typeMap.find(shader);
    return it != typeMap.end() ? it->second : "";
}