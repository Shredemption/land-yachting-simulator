#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

class Shader
{
public:
    void use();
    unsigned int m_id;

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setIntArray(const std::string &name, const int *values, size_t count) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setMat4Array(const std::string &name, const std::vector<glm::mat4> &mats) const;

    void init(const std::string &vertexCode, const std::string &fragmentCode);

private:
    unsigned int m_vertexId;
    unsigned int m_fragmentId;

    std::string m_vertexCode;
    std::string m_fragmentCode;

    void compile();
    void link();

    void checkCompileError(unsigned int shader, const std::string type);
    void checkLinkingError();
};