#pragma once

#include <string>

enum class EngineState;
enum class SettingsPage;
class Slider;

namespace UIManager
{
    inline bool needsReload = false;
    inline bool needsRestart = false;
    inline Slider *draggingSlider = nullptr;

    void countOptions(SettingsPage page);
    void load(EngineState state);
    void update();
    void render();

    void queueEngineState(EngineState state);
    void queueEngineScene(std::string scene);
    void queueSettingsPage(SettingsPage page);
};