#pragma once

#include <string>
#include <vector>
#include <map>

enum class ModelType
{
    Model,
    Yacht
};

struct JSONModelMapEntry
{
    std::string mainPath;
    std::vector<std::string> lodPaths = {};
    std::string type = "model";
};

struct JSONModelMap
{
    std::map<std::string, JSONModelMapEntry> yachts;
    std::map<std::string, JSONModelMapEntry> models;
};