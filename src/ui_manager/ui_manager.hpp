#pragma once

#include <string>

enum class EngineState;

namespace UIManager
{
    void load(EngineState state);
    void updateHTML();
    void loadHTML(const std::string file);
};