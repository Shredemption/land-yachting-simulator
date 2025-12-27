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
    inline std::map<std::string, ModelMapEntry> modelMap;
    void initModelMap();
};