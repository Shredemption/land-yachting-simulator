#pragma once

#include <atomic>
#include <string>
#include <map>

#include "model/model_defs.h"

namespace ModelUtil
{
    inline std::atomic<int> activeBoneBuffer{0};
    void swapBoneBuffers();

    // Model map and load function
    inline std::map<std::string, JSONModelMapEntry> modelMap;
    inline std::string modelMapPath = "resources/models.json";
    void loadModelMap();
};