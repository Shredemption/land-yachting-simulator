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
    std::vector<std::string> buttonTexts = {};
    std::vector<std::function<void()>> buttonFunctions = {};

    switch (state)
    {
    case EngineState::Title:

        buttonTexts = {
            "[1] Load Realistic Scene",
            "[2] Load Cartoon Scene",
            "[T] Load Test Scene",
            "",
            "[ESC] Quit"};

        buttonFunctions = {
            []
            { SceneManager::switchEngineStateScene("realistic"); },
            []
            { SceneManager::switchEngineStateScene("cartoon"); },
            []
            { SceneManager::switchEngineStateScene("test"); },
            []
            { return; },
            []
            { SceneManager::switchEngineState(EngineState::None); }};

        addButtonLine(glm::vec2(0.03, 0.20), glm::vec2(0.0, 0.05), glm::vec2(0.28, 0.03), buttonTexts,
                      0.5, glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.6, 0.6, 0.6), buttonFunctions);
        break;

    case EngineState::Pause:

        buttonTexts = {
            "[ESC] Resume",
            "[Q] Exit to Menu"};

        buttonFunctions = {
            []
            { SceneManager::switchEngineState(EngineState::Running); },
            []
            { SceneManager::switchEngineState(EngineState::Title); }};

        addButtonLine(glm::vec2(0.03, 0.20), glm::vec2(0.0, 0.05), glm::vec2(0.28, 0.03), buttonTexts,
                      0.5, glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.6, 0.6, 0.6), buttonFunctions);
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

void UIManager::addButtonLine(glm::vec2 startPos, glm::vec2 stepPos, glm::vec2 size, std::vector<std::string> texts,
                              float scale, glm::vec3 baseColor, glm::vec3 hoverColor, std::vector<std::function<void()>> callbacks)
{
    for (int i = 0; i < texts.size(); i++)
    {
        glm::vec2 pos = startPos + (float(i) * stepPos);

        buttons.push_back(UIButton(pos, size, texts[i], scale, baseColor, hoverColor));
        buttons.back().setOnClick(callbacks[i]);
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