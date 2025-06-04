#include "shader/shader.h"

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "file_manager/file_manager.h"

std::unordered_map<shaderID, Shader> Shader::loadedShaders;
shaderID Shader::lastShader;
bool Shader::waterLoaded = false;

Shader *Shader::load(const shaderID shaderID)
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
            shader.init(FileManager::read("shaders/" + NameFromShader(shaderID) + ".vs"), FileManager::read("shaders/" + NameFromShader(shaderID) + ".fs"));
            loadedShaders.emplace(shaderID, shader);
        }

        lastShader = shaderID;
        shaderPtr = &loadedShaders[shaderID];
        shaderPtr->use();
    }
    return shaderPtr;
}

void Shader::init(const std::string &vertexCode, const std::string &fragmentCode)
{
    m_vertexCode = vertexCode;
    m_fragmentCode = fragmentCode;
    compile();
    link();
}

void Shader::use()
{
    glUseProgram(m_id);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setIntArray(const std::string &name, const int *value, size_t count) const
{
    glUniform1iv(glGetUniformLocation(m_id, name.c_str()), count, value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4Array(const std::string &name, const std::vector<glm::mat4> &mats) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), mats.size(), GL_FALSE, glm::value_ptr(mats[0]));
}

void Shader::compile()
{
    const char *vsCode = m_vertexCode.c_str();
    m_vertexId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexId, 1, &vsCode, NULL);
    glCompileShader(m_vertexId);
    checkCompileError(m_vertexId, "Vertex Shader");

    const char *fsCode = m_fragmentCode.c_str();
    m_fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentId, 1, &fsCode, NULL);
    glCompileShader(m_fragmentId);
    checkCompileError(m_fragmentId, "Fragment Shader");
}

void Shader::link()
{
    m_id = glCreateProgram();
    glAttachShader(m_id, m_vertexId);
    glAttachShader(m_id, m_fragmentId);
    glLinkProgram(m_id);
    checkLinkingError();
    glDeleteShader(m_vertexId);
    glDeleteShader(m_fragmentId);
}

void Shader::checkCompileError(unsigned int shader, const std::string type)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "Shader: Error compiling " << type << ":" << std::endl
                  << infoLog << std::endl;
    }
}

void Shader::checkLinkingError()
{
    int success;
    char infoLog[1024];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_id, 1024, NULL, infoLog);
        std::cout << " Shader: Error linking shader program: " << std::endl
                  << infoLog << std::endl;
    }
}

void Shader::unload()
{
    // Clear global data from loading before loading new scene
    for (auto &shaderPair : Shader::loadedShaders)
    {
        glDeleteProgram(shaderPair.second.m_id); // Explicitly delete the shader program from the GPU
    }

    loadedShaders.clear();
    lastShader = shaderID::shNone;
}

shaderID Shader::ShaderFromName(const std::string shaderName)
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
        {"pause", shaderID::shPause},
        {"post", shaderID::shPost}};

    auto it = typeMap.find(shaderName);
    return it != typeMap.end() ? it->second : shaderID::shNone;
}

std::string Shader::NameFromShader(const shaderID shader)
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
        {shaderID::shPause, "pause"},
        {shaderID::shPost, "post"}};

    auto it = typeMap.find(shader);
    return it != typeMap.end() ? it->second : "";
}