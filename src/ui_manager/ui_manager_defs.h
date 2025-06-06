#ifndef UI_MANAGER_DEFS_H
#define UI_MANAGER_DEFS_H

#include <glm/glm.hpp>

#include <string>
#include <functional>

enum class UIElementType
{
    uiButton,
    uiToggle,
};

struct UIElementData
{
    UIElementType type;
    std::string text;
    std::function<void()> callback;
    bool *toggleVariable;
};

#endif