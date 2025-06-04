#ifndef MODEL_DEFS_H
#define MODEL_DEFS_H

#include <string>
#include <vector>
#include <map>

enum class ModelType
{
    mtModel,
    mtYacht
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

#endif