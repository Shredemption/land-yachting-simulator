#pragma once

#include <string>

enum class EngineState;
enum class SettingsPage;

namespace UIManager
{
    inline bool needsReload = false;
    inline bool needsRestart = false;
    
    void countOptions(SettingsPage page);
    void load(EngineState state);
    void update();
    void render();

    void queueEngineState(EngineState state);
    void queueEngineScene(std::string scene);
    void queueSettingsPage(SettingsPage page);
};