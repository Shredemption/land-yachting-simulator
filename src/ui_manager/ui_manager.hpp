#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>
#include <variant>

#include <ui_manager/ui_button.h>
#include <ui_manager/ui_toggle.h>

enum class EngineState;
enum class SettingsPage;

using UIElement = std::variant<UIButton*, UIToggle*>;

namespace UIManager
{
    void update();
    void load(const EngineState &state);
    void load(const SettingsPage &page);

    void draw();

    inline std::vector<UIElement> uiElements;
    inline std::vector<UIButton> buttons;
    inline std::vector<UIToggle> toggles;

    inline int selected = -1;

    inline glm::vec3 defaultBaseColor = {1.0f, 1.0f, 1.0f};
    inline glm::vec3 defaultHoverColor = {0.9f, 0.5f, 0.5f};
}

#endif