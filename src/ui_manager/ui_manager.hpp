#pragma once

#include <vector>
#include <variant>

#include <ui_manager/ui_button.h>
#include <ui_manager/ui_toggle.h>
#include <ui_manager/ui_manager_defs.h>

enum class EngineState;
enum class SettingsPage;

using UIElement = std::variant<UIButton *, UIToggle *>;

struct UIElementRef
{
    UIElement *element;
    UIInputState linkedState;
    int localIndex;
};

namespace UIManager
{
    void update();
    void load(const EngineState &state);
    void loadSide(const SettingsPage &page);

    void draw();

    inline std::vector<UIElementRef> uiElementsTotal;
    void rebuildTotalElements();
    UIElement *getSelectedElement();

    inline std::vector<UIElement> uiElements;
    inline std::vector<UIButton> buttons;
    inline std::vector<UIToggle> toggles;

    inline std::vector<UIElement> uiElementsSide;
    inline std::vector<UIButton> buttonsSide;
    inline std::vector<UIToggle> togglesSide;

    inline int selectedMain = -1;
    inline int selectedSide = -1;
    inline UIInputState inputState = UIInputState::uiMain;

    inline glm::vec3 defaultBaseColor = {1.0f, 1.0f, 1.0f};
    inline glm::vec3 defaultHoverColor = {0.9f, 0.4f, 0.4f};
    inline glm::vec3 defaultActiveColor = {0.6f, 0.1f, 0.1f};
};