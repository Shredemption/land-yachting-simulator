#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>

#include <ui_manager/ui_button.h>

enum class EngineState;

namespace UIManager
{
    void update();
    void load(EngineState state);
    void draw();

    inline std::vector<UIButton> buttons;
}

#endif