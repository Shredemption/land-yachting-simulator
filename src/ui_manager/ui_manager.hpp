#pragma once

#include <vector>
#include <variant>

#include "ui_manager/ui_manager_defs.h"

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
    void loadElements(std::vector<UIElementData> elementData, glm::vec2 startPos, glm::vec2 stepPos, glm::vec2 size, float scale, glm::vec3 baseCol, glm::vec3 hoverCol, glm::vec3 activeCol,
                      std::vector<UIElement> &UIelements, std::vector<UIButton> &UIbuttons, std::vector<UIToggle> &UItoggles);
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
    inline UIInputState inputState = UIInputState::Main;

    inline glm::vec3 defaultBaseColor = {1.0f, 1.0f, 1.0f};
    inline glm::vec3 defaultHoverColor = {0.9f, 0.4f, 0.4f};
    inline glm::vec3 defaultActiveColor = {0.6f, 0.1f, 0.1f};
};