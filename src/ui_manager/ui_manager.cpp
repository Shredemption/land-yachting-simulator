#include "ui_manager/ui_manager.hpp"
#include "ui_manager/ui_button.h"
#include "ui_manager/ui_toggle.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "camera/camera.hpp"
#include "debug/debug.hpp"
#include "event_handler/event_handler.hpp"
#include "physics/physics_util.hpp"
#include "render/render.hpp"
#include "scene_manager/scene_manager.hpp"
#include "scene_manager/scene_manager_defs.h"
#include "shader/shader_util.hpp"
#include "shader/shaderID.h"
#include "ui_manager/ui_manager_defs.h"

#include "iostream"

void UIManager::update()
{
    draw();

    for (auto button : buttons)
    {
        button.checkClicked(EventHandler::mousePosX, EventHandler::mousePosY, EventHandler::leftMouseButton.pressed());
    }
    for (auto toggle : toggles)
    {
        toggle.checkClicked(EventHandler::mousePosX, EventHandler::mousePosY, EventHandler::leftMouseButton.pressed());
    }
}

void UIManager::load(const EngineState &state)
{
    selected = -1;
    buttons.clear();
    toggles.clear();
    uiElements.clear();

    std::vector<UIElementData> elements = {};

    float scale = 0.5;
    glm::vec3 baseColor = defaultBaseColor;
    glm::vec3 hoverColor = defaultHoverColor;

    glm::vec2 startPos(0.03, 0.20);
    glm::vec2 stepPos(0.0, 0.05);

    glm::vec2 size(0.3, scale / 10);

    switch (state)
    {
    case EngineState::esTitle:

        elements = {
            {
                UIElementType::uiButton,
                "Load Realistic Scene",
                []
                { SceneManager::switchEngineStateScene("realistic"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Load Cartoon Scene",
                []
                { SceneManager::switchEngineStateScene("cartoon"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Load Test Scene",
                []
                { SceneManager::switchEngineStateScene("test"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::esTitleSettings); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Quit",
                []
                { SceneManager::switchEngineState(EngineState::esNone); },
                nullptr,
            },
        };

        break;

    case EngineState::esPause:

        elements = {
            {
                UIElementType::uiButton,
                "Resume",
                []
                { SceneManager::switchEngineState(EngineState::esRunning); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Restart",
                []
                {
                    Camera::reset();
                    PhysicsUtil::setup();
                    SceneManager::runOneFrame();
                    SceneManager::switchEngineState(EngineState::esRunning);
                },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::esSettings); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Exit to Menu",
                []
                { SceneManager::switchEngineState(EngineState::esTitle); },
                nullptr,
            },
        };

        break;

    case EngineState::esSettings:
    case EngineState::esTitleSettings:
        load(SceneManager::settingsPage);
        break;
    }

    for (int i = 0; i < elements.size(); i++)
    {
        UIElementData element = elements[i];

        switch (element.type)
        {
        case UIElementType::uiButton:
            buttons.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseColor, hoverColor);
            buttons.back().setOnClick(element.callback);
            break;

        case UIElementType::uiToggle:
            toggles.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseColor, hoverColor);
            toggles.back().toggleVariable = element.toggleVariable;
            break;
        }
    }

    int buttonIndex = 0;
    int toggleIndex = 0;
    for (const auto &element : elements)
    {
        switch (element.type)
        {
        case UIElementType::uiButton:
            uiElements.push_back(&buttons[buttonIndex++]);
            break;
        case UIElementType::uiToggle:
            uiElements.push_back(&toggles[toggleIndex++]);
            break;
        }
    }
}

void UIManager::load(const SettingsPage &page)
{
    selected = -1;
    buttons.clear();
    toggles.clear();
    uiElements.clear();

    std::vector<UIElementData> elements = {};

    float scale = 0.5;
    glm::vec3 baseColor = defaultBaseColor;
    glm::vec3 hoverColor = defaultHoverColor;

    glm::vec2 startPos(0.03, 0.20);
    glm::vec2 stepPos(0.0, 0.05);

    glm::vec2 size(0.3, scale / 10);

    switch (page)
    {
    case SettingsPage::spStart:

        elements = {
            {
                UIElementType::uiButton,
                "Graphics",
                []
                { SceneManager::switchSettingsPage(SettingsPage::spGraphics); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Physics",
                []
                { SceneManager::switchSettingsPage(SettingsPage::spPhysics); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Back",
                []
                { if (SceneManager::engineState == EngineState::esSettings)
                    SceneManager::switchEngineState(EngineState::esPause);
                else
                    SceneManager::switchEngineState(EngineState::esTitle); },
                nullptr,
            },
        };

        break;

    case SettingsPage::spGraphics:

        elements = {
            {
                UIElementType::uiButton,
                "Back",
                []
                { SceneManager::switchSettingsPage(SettingsPage::spStart); },
                nullptr,
            },
        };

        break;

    case SettingsPage::spPhysics:

        elements = {
            {
                UIElementType::uiButton,
                "Back",
                []
                { SceneManager::switchSettingsPage(SettingsPage::spStart); },
                nullptr,
            },
        };

        break;

    case SettingsPage::spDebug:

        elements = {
            {
                UIElementType::uiToggle,
                "Wireframe",
                {},
                &Debug::wireMode,
            },
            {
                UIElementType::uiButton,
                "Back",
                []
                { SceneManager::switchSettingsPage(SettingsPage::spStart); },
                nullptr,
            },
        };

        break;
    }

    for (int i = 0; i < elements.size(); i++)
    {
        UIElementData element = elements[i];

        switch (element.type)
        {
        case UIElementType::uiButton:
            buttons.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseColor, hoverColor);
            buttons.back().setOnClick(element.callback);
            uiElements.push_back(&buttons.back());
            break;

        case UIElementType::uiToggle:
            toggles.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseColor, hoverColor);
            toggles.back().toggleVariable = element.toggleVariable;
            uiElements.push_back(&toggles.back());
            break;
        }
    }

    int buttonIndex = 0;
    int toggleIndex = 0;
    for (const auto &element : elements)
    {
        switch (element.type)
        {
        case UIElementType::uiButton:
            uiElements.push_back(&buttons[buttonIndex++]);
            break;
        case UIElementType::uiToggle:
            uiElements.push_back(&toggles[toggleIndex++]);
            break;
        }
    }
}

void UIManager::draw()
{
    for (int i = 0; i < uiElements.size(); i++)
    {
        std::visit([i](auto *element)
                   {if (!element) return; 
                    element->draw(i == selected, EventHandler::inputType, EventHandler::mousePosX, EventHandler::mousePosY); },
                   uiElements[i]);
    }
}

bool UIButton::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * EventHandler::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * EventHandler::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.01f) * EventHandler::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.01f + size.y) * EventHandler::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIButton::draw(bool selected, InputType inputType, float mouseX, float mouseY)
{
    glm::vec3 color = baseColor;

    if (inputType == InputType::itMouse)
        color = isHovered(mouseX, mouseY) ? hoverColor : baseColor;
    else if (selected)
        color = hoverColor;

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text, x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text, x, y, scale, color, alpha);
    glEnable(GL_DEPTH_TEST);
}

bool UIToggle::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * EventHandler::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * EventHandler::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.01f) * EventHandler::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.01f + size.y) * EventHandler::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIToggle::draw(bool selected, InputType inputType, float mouseX, float mouseY)
{
    glm::vec3 color = baseColor;

    if (inputType == InputType::itMouse)
        color = isHovered(mouseX, mouseY) ? hoverColor : baseColor;
    else if (selected)
        color = hoverColor;

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text + (*toggleVariable ? ": True" : ": False"), x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text + (*toggleVariable ? ": True" : ": False"), x, y, scale, color, alpha);
    glEnable(GL_DEPTH_TEST);
}