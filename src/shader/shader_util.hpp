#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

#include "shader/shader.hpp"

enum class shaderID;

namespace ShaderUtil
{
    Shader *load(const shaderID shaderID);
    void unload();

    shaderID ShaderFromName(const std::string shaderName);
    std::string NameFromShader(const shaderID shader);

    inline std::unordered_map<shaderID, Shader> loadedShaders;

    inline shaderID lastShader;
    inline bool waterLoaded = false;
};