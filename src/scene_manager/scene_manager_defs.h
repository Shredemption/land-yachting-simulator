#pragma once

#include <string>
#include <functional>
#include <unordered_map>

struct LoadingStep
{
    std::string completedLabel;
    std::function<std::string()> activeMessage;
};

enum class EngineState
{
    None,
    Loading,
    Title,
    Pause,
    Settings,
    TitleSettings,
    Running
};

inline EngineState parseEngineState(const std::string &name)
{
    static const std::unordered_map<std::string, EngineState> map = {
        {"None", EngineState::None},
        {"Loading", EngineState::Loading},
        {"Title", EngineState::Title},
        {"Pause", EngineState::Pause},
        {"Settings", EngineState::Settings},
        {"TitleSettings", EngineState::TitleSettings},
        {"Running", EngineState::Running},
    };
    auto it = map.find(name);
    return it != map.end() ? it->second : EngineState::None;
}

enum class SettingsPage
{
    None,
    Start,
    Graphics,
    Debug
};