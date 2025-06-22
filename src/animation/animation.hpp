#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <map>

struct ModelData;

namespace Animation
{
    void update(ModelData &ModelData, const float &alpha);
    void update(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones);

    void updateGeneric(ModelData &ModelData, const float &alpha);
    void updateYachtBones(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones);
};