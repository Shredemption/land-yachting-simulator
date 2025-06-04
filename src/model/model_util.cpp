#include "model/model_util.hpp"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "model/model_defs.h"

std::string modelMapPath = "resources/models.json";

// Json setups
JSONCONS_N_MEMBER_TRAITS(JSONModelMapEntry, 1, mainPath, lodPaths, type);
JSONCONS_N_MEMBER_TRAITS(JSONModelMap, 0, models, yachts);

void ModelUtil::loadModelMap()
{
    const std::string path = "../" + modelMapPath;
    // Check if the file exists
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File not found: " + path);
    }

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path);
    }

    JSONModelMap jsonModelMap = jsoncons::decode_json<JSONModelMap>(file);

    // If model classified as yacht
    for (const auto &[name, data] : jsonModelMap.yachts)
    {
        modelMap[name] = data;
    }
    // If geneneric model
    for (const auto &[name, data] : jsonModelMap.models)
    {
        modelMap[name] = data;
    }
}

void ModelUtil::swapBoneBuffers()
{
    int oldIndex = activeBoneBuffer.load(std::memory_order_relaxed);
    activeBoneBuffer.store(1 - oldIndex, std::memory_order_release);
}
