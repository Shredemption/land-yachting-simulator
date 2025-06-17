#include "ui_manager/widget.hpp"

#include "pch.h"

void Widget::Render()
{
    for (auto &widget : children)
    {
        widget->Render();
    }
}

void Widget::Update()
{
    for (auto &widget : children)
    {
        widget->Update();
    }
}

void Widget::AddChild(std::shared_ptr<Widget> child)
{
    children.push_back(child);
}

void Button::Render()
{
    glm::vec3 currentColor = active ? activeColor : (hover ? hoverColor : color);

    Render::renderText(text, pos.x + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Left);
    Render::renderText(text, pos.x, pos.y + 0.01f * scale, scale, currentColor, alpha, TextAlign::Left);
}

void Button::Update()
{
    float xmin = WindowManager::screenUIScale * 2560.0f * (pos.x);
    float xmax = WindowManager::screenUIScale * 2560.0f * (pos.x + size.x);
    float ymin = WindowManager::screenUIScale * 1440.0f * (pos.y);
    float ymax = WindowManager::screenUIScale * 1440.0f * (pos.y + size.y);

    if (linkedPage == SceneManager::settingsPage && linkedPage != SettingsPage::None)
        active = true;
    else if (InputManager::mousePosX > xmin && InputManager::mousePosX < xmax && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
        hover = true;
    else
    {
        active = false;
        hover = false;
    }

    if (hover && InputManager::leftMouseButton.released())
        onClick();
}

void Toggle::Render()
{
    if (hidden)
        return;

    glm::vec3 currentColor = hover ? hoverColor : color;

    Render::renderText(text + ": " + (*linkedVariable ? trueLabel : falseLabel), pos.x + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Left);
    Render::renderText(text + ": " + (*linkedVariable ? trueLabel : falseLabel), pos.x, pos.y + 0.01f * scale, scale, currentColor, alpha, TextAlign::Left);
}

void Toggle::Update()
{
    float xmin = WindowManager::screenUIScale * 2560.0f * (pos.x);
    float xmax = WindowManager::screenUIScale * 2560.0f * (pos.x + size.x);
    float ymin = WindowManager::screenUIScale * 1440.0f * (pos.y);
    float ymax = WindowManager::screenUIScale * 1440.0f * (pos.y + size.y);

    if (linkedPage == SceneManager::settingsPage)
    {
        hidden = false;
        if (InputManager::mousePosX > xmin && InputManager::mousePosX < xmax && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hover = true;
        else
            hover = false;
    }
    else
    {
        hidden = true;
        hover = false;
    }

    if (hover && InputManager::leftMouseButton.released())
    {
        *linkedVariable = !(*linkedVariable);
        if (onChange)
            onChange();
    }
}