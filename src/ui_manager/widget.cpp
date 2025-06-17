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
    auto snapshot = children;

    for (auto &widget : snapshot)
    {
        if (widget)
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

    if (activePage == SceneManager::settingsPage && activePage != SettingsPage::None)
        active = true;
    else
        active = false;

    switch (InputManager::inputType)
    {
    case InputType::Mouse:
    {
        if (InputManager::mousePosX > xmin && InputManager::mousePosX < xmax && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hover = true;
        else
        {
            hover = false;
        }

        if (hover && InputManager::leftMouseButton.released())
            Execute();

        break;
    }
    case InputType::Keyboard:
    {
        if (index == UIManager::selected && (SceneManager::settingsPage == SettingsPage::Start || SceneManager::settingsPage == SettingsPage::None))
            hover = true;
        else
            hover = false;

        if (hover && UIManager::trigger)
            Execute();

        break;
    }
    }
}

void Button::Execute()
{
    if (onClick)
        onClick();
}

void Toggle::Render()
{
    if (hidden)
        return;

    Render::renderText(text + ":", pos.x + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Left);
    Render::renderText(text + ":", pos.x, pos.y + 0.01f * scale, scale, color, alpha, TextAlign::Left);

    Render::renderText("<", pos.x + leftOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText("<", pos.x + leftOffset, pos.y + 0.01f * scale, scale, hoverLeft ? hoverColor : color, alpha, TextAlign::Center);

    Render::renderText(*linkedVariable ? trueLabel : falseLabel, pos.x + labelOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText(*linkedVariable ? trueLabel : falseLabel, pos.x + labelOffset, pos.y + 0.01f * scale, scale, color, alpha, TextAlign::Center);

    Render::renderText(">", pos.x + rightOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText(">", pos.x + rightOffset, pos.y + 0.01f * scale, scale, hoverRight ? hoverColor : color, alpha, TextAlign::Center);
}

void Toggle::Update()
{
    float xminLeft = WindowManager::screenUIScale * 2560.0f * (pos.x + leftOffset - 0.01f);
    float xmaxLeft = WindowManager::screenUIScale * 2560.0f * (pos.x + leftOffset + 0.01f);
    float xminRight = WindowManager::screenUIScale * 2560.0f * (pos.x + rightOffset - 0.01f);
    float xmaxRight = WindowManager::screenUIScale * 2560.0f * (pos.x + rightOffset + 0.01f);
    float ymin = WindowManager::screenUIScale * 1440.0f * (pos.y);
    float ymax = WindowManager::screenUIScale * 1440.0f * (pos.y + size.y);

    if (shownOnPage == SceneManager::settingsPage)
        hidden = false;
    else
    {
        hidden = true;
        return;
    }

    switch (InputManager::inputType)
    {
    case InputType::Mouse:
    {
        if (InputManager::mousePosX > xminLeft && InputManager::mousePosX < xmaxLeft && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hoverLeft = true;
        else
            hoverLeft = false;

        if (InputManager::mousePosX > xminRight && InputManager::mousePosX < xmaxRight && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hoverRight = true;
        else
            hoverRight = false;

        if (InputManager::leftMouseButton.released())
        {
            if (hoverLeft)
                Execute(false);

            if (hoverRight)
                Execute(true);
        }

        break;
    }
    case InputType::Keyboard:
    {
        if (!hidden && index == UIManager::selected)
        {
            hoverLeft = true;
            hoverRight = true;
        }
        else
        {
            hoverLeft = false;
            hoverRight = false;
        }

        if (hoverLeft && UIManager::triggerLeft)
            Execute(false);

        if (hoverRight && UIManager::triggerRight)
            Execute(true);
    }
    break;
    }
}

void Toggle::Execute(bool toRight)
{
    *linkedVariable = !(*linkedVariable);

    if (onChange)
        onChange();
}

void Selector::Render()
{
    if (hidden)
        return;

    Render::renderText(text + ":", pos.x + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Left);
    Render::renderText(text + ":", pos.x, pos.y + 0.01f * scale, scale, color, alpha, TextAlign::Left);

    Render::renderText("<", pos.x + leftOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText("<", pos.x + leftOffset, pos.y + 0.01f * scale, scale, hoverLeft ? hoverColor : color, alpha, TextAlign::Center);

    Render::renderText(labels[currentIndex], pos.x + labelOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText(labels[currentIndex], pos.x + labelOffset, pos.y + 0.01f * scale, scale, color, alpha, TextAlign::Center);

    Render::renderText(">", pos.x + rightOffset + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Center);
    Render::renderText(">", pos.x + rightOffset, pos.y + 0.01f * scale, scale, hoverRight ? hoverColor : color, alpha, TextAlign::Center);
}

void Selector::Update()
{
    float xminLeft = WindowManager::screenUIScale * 2560.0f * (pos.x + leftOffset - 0.01f);
    float xmaxLeft = WindowManager::screenUIScale * 2560.0f * (pos.x + leftOffset + 0.01f);
    float xminRight = WindowManager::screenUIScale * 2560.0f * (pos.x + rightOffset - 0.01f);
    float xmaxRight = WindowManager::screenUIScale * 2560.0f * (pos.x + rightOffset + 0.01f);
    float ymin = WindowManager::screenUIScale * 1440.0f * (pos.y);
    float ymax = WindowManager::screenUIScale * 1440.0f * (pos.y + size.y);

    if (shownOnPage == SceneManager::settingsPage)
        hidden = false;
    else
    {
        hidden = true;
        return;
    }

    switch (InputManager::inputType)
    {
    case InputType::Mouse:
    {
        if (InputManager::mousePosX > xminLeft && InputManager::mousePosX < xmaxLeft && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hoverLeft = true;
        else
            hoverLeft = false;

        if (InputManager::mousePosX > xminRight && InputManager::mousePosX < xmaxRight && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
            hoverRight = true;
        else
            hoverRight = false;

        if (InputManager::leftMouseButton.released())
        {
            bool changed = false;
            if (hoverLeft)
                Execute(false);
            if (hoverRight)
                Execute(true);
        }

        break;
    }
    case InputType::Keyboard:
    {
        if (!hidden && index == UIManager::selected)
        {
            hoverLeft = true;
            hoverRight = true;
        }
        else
        {
            hoverLeft = false;
            hoverRight = false;
        }

        if (hoverLeft && UIManager::triggerLeft)
            Execute(false);

        if (hoverRight && UIManager::triggerRight)
            Execute(true);
    }
    break;
    }
}

void Selector::Execute(bool toRight)
{
    if (toRight)
        currentIndex = (currentIndex + 1 + labels.size()) % labels.size();
    else
        currentIndex = (currentIndex - 1 + labels.size()) % labels.size();

    if (onChange)
        onChange();
}
