#ifndef ANIMATION_H
#define ANIMATION_H

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <map>

struct ModelData;

class Animation
{
public:
    static void updateYachtBones(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones);
    static std::unordered_map<std::string, std::map<std::string, int>> yachtBoneMap;
};

#endif