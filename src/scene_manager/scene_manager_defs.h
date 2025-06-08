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
    esNone,
    esLoading,
    esTitle,
    esPause,
    esSettings,
    esTitleSettings,
    esRunning
};

enum class SettingsPage
{
    spNone,
    spStart,
    spGraphics,
    spDebug
};