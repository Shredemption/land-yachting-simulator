#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <optional>

#include "ui_manager/ui_button.hpp"
#include "ui_manager/ui_toggle.hpp"
#include "ui_manager/ui_selector.hpp"

enum class SettingsPage;

enum class UIElementType
{
    Button,
    Toggle,
    Selector,
};

struct UIElementData
{
    UIElementType type;
    std::string text;
    std::function<void()> callback;
    bool *toggleVariable;
    std::optional<SettingsPage> linkedPage;
    std::vector<std::string> optionLabels;
    std::function<void(UISelector &)> readCallback;
    std::function<void(UISelector &)> writeCallback;
};

enum class UIInputState
{
    Main,
    Side,
};