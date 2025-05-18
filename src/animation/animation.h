#ifndef ANIMATION_H
#define ANIMATION_H

#include <unordered_map>
#include <iostream>

#include "scene/scene.h"

class Animation
{
public:
    static void updateYachtBones(ModelData &ModelData);
    static std::unordered_map<std::string, std::map<std::string, int>> yachtBoneMap;
};

#endif