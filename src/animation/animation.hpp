#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <map>

struct ModelData;

namespace Animation
{
    void updateYachtBones(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones);
};

#endif