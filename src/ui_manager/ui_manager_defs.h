#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <optional>

enum class SettingsPage;

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
    std::optional<SettingsPage> linkedPage;
};

enum class UIInputState
{
    uiMain,
    uiSide,
};