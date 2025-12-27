#pragma once

#include <string>
#include <vector>
#include <map>

enum class ModelType
{
    Model,
    Yacht
};

struct ModelMapEntry
{
    std::string mainPath;
    std::vector<std::string> lodPaths = {};
    std::string hitboxPath = "";
    std::string type = "model";
};