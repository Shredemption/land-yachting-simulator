#ifndef SCENE_MANAGER_DEFS_H
#define SCENE_MANAGER_DEFS_H

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
    esRunning
};

enum class SettingsPage
{
    spStart,
    spDebug
};

#endif