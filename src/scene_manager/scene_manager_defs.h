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
    Boot,
    Loading,
    Title,
    Pause,
    Settings,
    TitleSettings,
    TestMenu,
    Running
};

enum class SettingsPage
{
    None,
    Start,
    Graphics,
    Input,
    Physics,
    Debug
};