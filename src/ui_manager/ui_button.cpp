#include "ui_manager/ui_button.hpp"

#include "pch.h"

void UIButton::checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
{
    if (mousePressed && isHovered(mouseX, mouseY))
        if (onClick)
            onClick();
}

bool UIButton::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * WindowManager::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * WindowManager::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.005f) * WindowManager::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.005f + size.y) * WindowManager::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIButton::draw(bool selected, bool active, InputType inputType, float mouseX, float mouseY)
{
    glm::vec3 color = baseColor;

    if (inputType == InputType::Mouse)
        color = isHovered(mouseX, mouseY) ? hoverColor : active ? activeColor
                                                                : baseColor;
    else
        color = selected ? hoverColor : active ? activeColor
                                               : baseColor;

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text, x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text, x, y, scale, color, alpha);
    glEnable(GL_DEPTH_TEST);
}

void UIButton::setOnClick(std::function<void()> callback)
{
    this->onClick = callback;
}

void UIButton::setOffset(glm::vec2 offset)
{
    this->offset = offset;
}

void UIButton::setAlpha(float alpha)
{
    this->alpha = alpha;
}