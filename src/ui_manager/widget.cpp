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
    glm::vec3 activeColor = hover ? hoverColor : color;

    Render::renderText(text, pos.x + 0.003f * scale, pos.y + (0.01f + 0.003f) * scale, scale, glm::vec3(0.0f), alpha, TextAlign::Left);
    Render::renderText(text, pos.x, pos.y + 0.01f * scale, scale, activeColor, alpha, TextAlign::Left);
}

void Button::Update()
{
    float xmin = WindowManager::screenWidth * (pos.x);
    float xmax = WindowManager::screenWidth * (pos.x + size.x);
    float ymin = WindowManager::screenHeight * (pos.y);
    float ymax = WindowManager::screenHeight * (pos.y + size.y);

    if (InputManager::mousePosX > xmin && InputManager::mousePosX < xmax && InputManager::mousePosY > ymin && InputManager::mousePosY < ymax)
        hover = true;
    else
        hover = false;

    if (hover && InputManager::leftMouseButton.released())
        onClick();
}