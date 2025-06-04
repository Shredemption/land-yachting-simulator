#include "ui_manager/ui_manager.hpp"
#include "ui_manager/ui_button.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "event_handler/event_handler.hpp"
#include "render/render.hpp"
#include "scene_manager/scene_manager.hpp"
#include "scene_manager/scene_manager_defs.h"
#include "shader/shader_util.hpp"
#include "shader/shaderID.h"

#include "iostream"

void UIManager::update()
{
    draw();

    for (auto button : buttons)
    {
        button.checkClicked(EventHandler::mousePosX, EventHandler::mousePosY, EventHandler::leftMouseButton.pressed());
    }
}

void UIManager::load(EngineState state)
{
    buttons.clear();

    std::vector<ButtonData> buttonSet = {};

    switch (state)
    {
    case EngineState::Title:

        buttonSet = {
            {"[1] Load Realistic Scene", {0.03, 0.20}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineStateScene("realistic"); }},
            {"[2] Load Cartoon Scene", {0.03, 0.25}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineStateScene("cartoon"); }},
            {"[T] Load Test Scene", {0.03, 0.30}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineStateScene("test"); }},
            {"[ESC] Quit", {0.03, 0.40}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineState(EngineState::None); }}};
        break;

    case EngineState::Pause:

        buttonSet = {
            {"[ESC] Resume", {0.03, 0.20}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineState(EngineState::Running); }},
            {"[Q] Exit to Menu", {0.03, 0.25}, {0.27, 0.03}, 0.5, []
             { SceneManager::switchEngineState(EngineState::Title); }}};
        break;
    }

    for (auto button : buttonSet)
    {
        buttons.push_back(UIButton(button.pos, button.size, button.text, button.scale));
        buttons.back().setOnClick(button.callback);
    }
}

void UIManager::draw()
{
    for (auto button : buttons)
    {
        glm::vec3 color = button.isHovered(EventHandler::mousePosX, EventHandler::mousePosY) ? button.hoverColor : button.baseColor;

        Render::renderText(button.text, button.pos.x, button.pos.y, button.scale, color);
    }
}

bool UIButton::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * EventHandler::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * EventHandler::screenUIScale * 2560.0f;

    float ymin = pos.y * EventHandler::screenUIScale * 1440.0f;
    float ymax = (pos.y + size.y) * EventHandler::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIButton::checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
{
    if (mousePressed && isHovered(mouseX, mouseY))
        if (onClick)
            onClick();
}

void UIButton::setOnClick(std::function<void()> callback)
{
    onClick = callback;
};