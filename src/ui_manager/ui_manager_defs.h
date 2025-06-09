#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <optional>

#include "ui_manager/ui_button.hpp"
#include "ui_manager/ui_toggle.hpp"

enum class SettingsPage;

enum class UIElementType
{
    Button,
    Toggle,
};

struct UIElementData
{
    UIElementType type;
    std::string text;
    std::function<void()> callback;
    bool *toggleVariable;
    std::optional<SettingsPage> linkedPage;
    std::string trueText;
    std::string falseText;
};

enum class UIInputState
{
    Main,
    Side,
};