#include "shader/shader_util.hpp"

#include "pch.h"

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
    lastShader = shaderID::None;

    waterLoaded = false;
}

shaderID ShaderUtil::ShaderFromName(const std::string shaderName)
{
    static const std::unordered_map<std::string, shaderID> typeMap = {
        {"default", shaderID::Default},
        {"gui", shaderID::Gui},
        {"simple", shaderID::Simple},
        {"text", shaderID::Text},
        {"image", shaderID::Image},
        {"water", shaderID::Water},
        {"skybox", shaderID::Skybox},
        {"toon", shaderID::Toon},
        {"toon-terrain", shaderID::ToonTerrain},
        {"toon-water", shaderID::ToonWater},
        {"darken-blur", shaderID::DarkenBlur},
        {"post", shaderID::Post},
    };

    auto it = typeMap.find(shaderName);
    return it != typeMap.end() ? it->second : shaderID::None;
}

std::string ShaderUtil::NameFromShader(const shaderID shader)
{
    static const std::unordered_map<shaderID, std::string> typeMap = {
        {shaderID::Default, "default"},
        {shaderID::Gui, "gui"},
        {shaderID::Simple, "simple"},
        {shaderID::Text, "text"},
        {shaderID::Image, "image"},
        {shaderID::Water, "water"},
        {shaderID::Skybox, "skybox"},
        {shaderID::Toon, "toon"},
        {shaderID::ToonTerrain, "toon-terrain"},
        {shaderID::ToonWater, "toon-water"},
        {shaderID::DarkenBlur, "darken-blur"},
        {shaderID::Post, "post"},
    };

    auto it = typeMap.find(shader);
    return it != typeMap.end() ? it->second : "";
}