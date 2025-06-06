#include "ui_manager/ui_manager.hpp"
#include "ui_manager/ui_button.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "camera/camera.hpp"
#include "event_handler/event_handler.hpp"
#include "physics/physics_util.hpp"
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

void UIManager::load(const EngineState &state)
{
    selected = -1;
    buttons.clear();

    std::vector<ButtonData> buttonSet = {};
    std::vector<std::string> buttonTexts = {};
    std::vector<std::function<void()>> buttonFunctions = {};

    float scale = 0.5;

    glm::vec2 startPos(0.03, 0.20);
    glm::vec2 stepPos(0.0, 0.05);

    glm::vec2 size(0.3, scale / 10);

    switch (state)
    {
    case EngineState::esTitle:

        buttonTexts = {
            "Load Realistic Scene",
            "Load Cartoon Scene",
            "Load Test Scene",
            "Settings",
            "Quit",
        };

        buttonFunctions = {
            []
            { SceneManager::switchEngineStateScene("realistic"); },
            []
            { SceneManager::switchEngineStateScene("cartoon"); },
            []
            { SceneManager::switchEngineStateScene("test"); },
            []
            { SceneManager::switchEngineState(EngineState::esTitleSettings); },
            []
            { SceneManager::switchEngineState(EngineState::esNone); },
        };

        break;

    case EngineState::esPause:

        buttonTexts = {
            "Resume",
            "Restart",
            "Settings",
            "Exit to Menu",
        };

        buttonFunctions = {
            []
            { SceneManager::switchEngineState(EngineState::esRunning); },
            []
            {
                Camera::reset();
                PhysicsUtil::setup();
                SceneManager::runOneFrame();
                SceneManager::switchEngineState(EngineState::esRunning);
            },
            []
            { SceneManager::switchEngineState(EngineState::esSettings); },
            []
            { SceneManager::switchEngineState(EngineState::esTitle); },
        };

        break;

    case EngineState::esSettings:
    case EngineState::esTitleSettings:
        load(SceneManager::settingsPage);
        break;
    }

    addButtonLine(startPos, stepPos, size, buttonTexts, scale, defaultBaseColor, defaultHoverColor, buttonFunctions);

    for (auto button : buttonSet)
    {
        buttons.push_back(UIButton(button.pos, button.size, button.text, button.scale, defaultBaseColor, defaultHoverColor));
        buttons.back().setOnClick(button.callback);
    }
}

void UIManager::load(const SettingsPage &page)
{
    selected = -1;
    buttons.clear();

    std::vector<ButtonData> buttonSet = {};
    std::vector<std::string> buttonTexts = {};
    std::vector<std::function<void()>> buttonFunctions = {};

    float scale = 0.5;

    glm::vec2 startPos(0.03, 0.20);
    glm::vec2 stepPos(0.0, 0.05);

    glm::vec2 size(0.3, scale / 10);

    switch (page)
    {
    case SettingsPage::spStart:

        buttonTexts = {
            "Graphics",
            "Physics",
            "Back",
        };

        buttonFunctions = {
            []
            { SceneManager::switchSettingsPage(SettingsPage::spGraphics); },
            []
            { SceneManager::switchSettingsPage(SettingsPage::spPhysics); },
            []
            {
                if (SceneManager::engineState == EngineState::esSettings)
                    SceneManager::switchEngineState(EngineState::esPause);
                else
                    SceneManager::switchEngineState(EngineState::esTitle);
            },
        };

        break;

    case SettingsPage::spGraphics:
        buttonTexts = {
            "Back",
        };

        buttonFunctions = {
            []
            { SceneManager::switchSettingsPage(SettingsPage::spStart); },
        };

        break;

    case SettingsPage::spPhysics:
        buttonTexts = {
            "Back",
        };

        buttonFunctions = {
            []
            { SceneManager::switchSettingsPage(SettingsPage::spStart); },
        };

        break;

    case SettingsPage::spDebug:
        buttonTexts = {
            "Back",
        };

        buttonFunctions = {
            []
            { SceneManager::switchSettingsPage(SettingsPage::spStart); },
        };

        break;
    }

    addButtonLine(startPos, stepPos, size, buttonTexts, scale, defaultBaseColor, defaultHoverColor, buttonFunctions);

    for (auto button : buttonSet)
    {
        buttons.push_back(UIButton(button.pos, button.size, button.text, button.scale, defaultBaseColor, defaultHoverColor));
        buttons.back().setOnClick(button.callback);
    }
}

void UIManager::draw()
{
    for (int i = 0; i < buttons.size(); i++)
    {
        UIButton button = buttons[i];

        glm::vec3 color;

        switch (EventHandler::inputType)
        {
        case InputType::itMouse:
            color = button.isHovered(EventHandler::mousePosX, EventHandler::mousePosY) ? button.hoverColor : button.baseColor;
            break;

        default:
            color = i == selected ? button.hoverColor : button.baseColor;
        }

        glDisable(GL_DEPTH_TEST);
        Render::renderText(button.text, button.pos.x + button.offset.x + 0.003f, button.pos.y + button.offset.y + 0.003f, button.scale, glm::vec3(0, 0, 0), button.alpha);
        Render::renderText(button.text, button.pos.x + button.offset.x, button.pos.y + button.offset.y, button.scale, color, button.alpha);
        glEnable(GL_DEPTH_TEST);
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

int UIManager::optionCount()
{
    return buttons.size();
}

bool UIButton::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * EventHandler::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * EventHandler::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.01f) * EventHandler::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.01f + size.y) * EventHandler::screenUIScale * 1440.0f;

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

void UIButton::setOffset(glm::vec2 offset)
{
    this->offset = offset;
}

void UIButton::setAlpha(float alpha)
{
    this->alpha = alpha;
}