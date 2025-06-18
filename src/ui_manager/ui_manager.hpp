#pragma once

#include <string>

enum class EngineState;
enum class SettingsPage;

namespace UIManager
{
    void countOptions(SettingsPage page);
    void load(EngineState state);
    void update();
    void render();

    void queueEngineState(EngineState state);
    void queueEngineScene(std::string scene);
};