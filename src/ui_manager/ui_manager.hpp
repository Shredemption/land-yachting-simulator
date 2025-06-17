#pragma once

enum class EngineState;

namespace UIManager
{
    void load(EngineState state);
    void update();
    void render();

    inline int selected;
    inline int options;
};