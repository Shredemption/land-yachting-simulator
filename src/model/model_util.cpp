#include "model/model_util.hpp"

#include "pch.h"

ModelMapEntry makeModel(
    std::string mainPath,
    std::vector<std::string> lodPaths = {},
    std::string hitboxPath = "",
    std::string type = "model")
{
    return ModelMapEntry{
        std::move(mainPath),
        std::move(lodPaths),
        std::move(hitboxPath),
        std::move(type)};
}

void ModelUtil::initModelMap()
{
    std::cout << "[ModelUtil] Initializing model registry" << std::endl;

    modelMap = {

        // -- yachts -- 
        
        {"dn-duvel",
         makeModel(
             "resources/models/yachts/dn-duvel/dn-duvel.dae",
             {"resources/models/yachts/dn-duvel/dn-duvel-lod1.dae"},
             "",
             "yacht")},
        {"bobbie",
         makeModel(
             "resources/models/yachts/bobbie/bobbie.dae",
             {"resources/models/yachts/bobbie/bobbie-lod1.dae"},
             "",
             "yacht")},
        {"vampier",
         makeModel(
             "resources/models/yachts/vampier/vampier.dae",
             {"resources/models/yachts/vampier/vampier-lod1.dae"},
             "",
             "yacht")},
        {"beware",
         makeModel(
             "resources/models/yachts/beware/beware.dae",
             {"resources/models/yachts/beware/beware-lod1.dae"},
             "",
             "yacht")},
        {"buizerd",
         makeModel(
             "resources/models/yachts/buizerd/buizerd.dae",
             {"resources/models/yachts/buizerd/buizerd-lod1.dae"},
             "",
             "yacht")},
        {"red-piper",
         makeModel(
             "resources/models/yachts/red-piper/red-piper.dae",
             {"resources/models/yachts/red-piper/red-piper-lod1.dae"},
             "",
             "yacht")},
        {"blue-piper",
         makeModel(
             "resources/models/yachts/blue-piper/blue-piper.dae",
             {"resources/models/yachts/blue-piper/blue-piper-lod1.dae"},
             "",
             "yacht")},
        {"sietske",
         makeModel(
             "resources/models/yachts/sietske/sietske.dae",
             {"resources/models/yachts/sietske/sietske-lod1.dae"},
             "",
             "yacht")},

        // ---- generic / test models ----

        {
            "cube",
            makeModel(
                "resources/models/test/cube.dae")},
        {"icosphere",
         makeModel(
             "resources/models/test/icosphere.dae")},
        {"cylinder",
         makeModel(
             "resources/models/test/cylinder.dae")}};

    std::cout << "[ModelUtil] Model registry initialized (" << modelMap.size() << " models)" << std::endl;
}

void ModelUtil::swapBoneBuffers()
{
    int oldIndex = activeBoneBuffer.load(std::memory_order_relaxed);
    activeBoneBuffer.store(1 - oldIndex, std::memory_order_release);
}
