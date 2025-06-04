#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>

#include <ui_manager/ui_button.h>

enum class EngineState;

namespace UIManager
{
    void update();
    void load(EngineState state);

    void addButtonLine(glm::vec2 startPos, glm::vec2 stepPos, glm::vec2 size, std::vector<std::string> texts,
                       float scale, glm::vec3 baseColor, glm::vec3 hoverColor, std::vector<std::function<void()>> callbacks);
    void draw();

    inline std::vector<UIButton> buttons;
}

#endif