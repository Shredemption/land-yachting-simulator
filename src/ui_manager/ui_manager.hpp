#pragma once

enum class EngineState;
enum class SettingsPage;

namespace UIManager
{
    void countOptions(SettingsPage page);
    void load(EngineState state);
    void update();
    void render();

    inline int selected;
    inline int options;
    inline bool trigger = false;
    inline bool triggerLeft = false;
    inline bool triggerRight = false;
};