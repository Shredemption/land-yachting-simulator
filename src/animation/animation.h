#include <unordered_map>
#include <iostream>

#include <scene/scene.h>

class Animation
{
public:
    static void updateBones(Scene &scene);
    static void generateYachtBones(Model *model);
    static std::unordered_map<std::string, std::map<std::string, int>> yachtBoneMap;
};