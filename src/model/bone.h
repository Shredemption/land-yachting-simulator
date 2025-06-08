#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Bone
{
    std::string name;
    int index;
    glm::mat4 offsetMatrix;
    glm::mat4 transform = glm::mat4(1.0f);
    Bone *parent;
    std::vector<Bone *> children;

    Bone(std::string boneName, int boneIndex, const glm::mat4 &offset, Bone *parentBone = nullptr)
        : name(boneName), index(boneIndex), offsetMatrix(offset), parent(parentBone) {}
};