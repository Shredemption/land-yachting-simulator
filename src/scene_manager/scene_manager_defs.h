#pragma once

#include <string>
#include <functional>

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

enum class SettingsPage
{
    None,
    Start,
    Graphics,
    Debug
};