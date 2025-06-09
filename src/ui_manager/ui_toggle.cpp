#include "ui_manager/ui_toggle.hpp"

#include "pch.h"

void UIToggle::checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
{
    glm::vec2 leftArrowPos = glm::vec2(pos.x + labelOffset - arrowSpacing, pos.y);
    glm::vec2 rightArrowPos = glm::vec2(pos.x + labelOffset + arrowSpacing, pos.y);

    glm::vec2 arrowSize = glm::vec2(scale / 20.0f, scale / 10.0f);

    hoverLeft = false;
    hoverRight = false;

    if (isInside(mouseX, mouseY, leftArrowPos, arrowSize))
    {
        hoverLeft = true;
        if (mousePressed)
            execute();
    }
    else if (isInside(mouseX, mouseY, rightArrowPos, arrowSize))
    {
        hoverRight = true;
        if (mousePressed)
            execute();
    }
}

void UIToggle::toggle()
{
    if (!toggleVariable)
        return;

    startAnimation(!(toggleVariable));
    *toggleVariable = !(*toggleVariable);
}

void UIToggle::execute()
{
    toggle();
    if (onClick)
        onClick();
}

void UIToggle::setTextOptions(const std::string &falseLabel, const std::string &trueLabel)
{
    falseText = falseLabel;
    trueText = trueLabel;
}

void UIToggle::startAnimation(bool toTrue)
{
    animDirection = toTrue;
    animTime = 0.0f;
    animating = true;
}

bool UIToggle::isInside(const float mouseX, const float mouseY, glm::vec2 pos, glm::vec2 size)
{
    float xmin = (pos.x - size.x / 2.0f) * WindowManager::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x / 2.0f) * WindowManager::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.005f) * WindowManager::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.005f + size.y) * WindowManager::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIToggle::draw(bool selected, InputType inputType, float mouseX, float mouseY)
{
    if (animating)
    {
        animTime += animSpeed * TimeManager::deltaTime;
        if (animTime >= 1.0f)
        {
            animTime = 1.0f;
            animating = false;
        }
    }

    const std::string &currentLabel = *toggleVariable ? trueText : falseText;
    const std::string &prevLabel = *toggleVariable ? falseText : trueText;

    float slide = 0.0f;
    if (animating)
    {
        float progress = animTime;
        slide = (animDirection ? (1.0f - progress) : progress) * slideDistance;
    }

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text, x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text, x, y, scale, baseColor, alpha);

    Render::renderText("<", x + labelOffset - arrowSpacing + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
    Render::renderText(">", x + labelOffset + arrowSpacing + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
    Render::renderText("<", x + labelOffset - arrowSpacing, y, scale, (selected || hoverLeft) ? hoverColor : baseColor, alpha, TextAlign::Center);
    Render::renderText(">", x + labelOffset + arrowSpacing, y, scale, (selected || hoverRight) ? hoverColor : baseColor, alpha, TextAlign::Center);

    if (animating)
    {
        float alphaA = std::clamp(1.0f - animTime * animTime, 0.0f, 1.0f);
        float alphaB = std::clamp(animTime * animTime, 0.0f, 1.0f);

        Render::renderText(prevLabel,
                           x + labelOffset + (animDirection ? -slide : slide) + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha * alphaA, TextAlign::Center);
        Render::renderText(currentLabel,
                           x + labelOffset + (animDirection ? slideDistance - slide : slide - slideDistance) + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha * alphaB, TextAlign::Center);

        Render::renderText(prevLabel,
                           x + labelOffset + (animDirection ? -slide : slide), y, scale, baseColor, alpha * alphaA, TextAlign::Center);
        Render::renderText(currentLabel,
                           x + labelOffset + (animDirection ? slideDistance - slide : slide - slideDistance), y, scale, baseColor, alpha * alphaB, TextAlign::Center);
    }
    else
    {
        Render::renderText(currentLabel, x + labelOffset + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
        Render::renderText(currentLabel, x + labelOffset, y, scale, baseColor, alpha, TextAlign::Center);
    }

    glEnable(GL_DEPTH_TEST);
}

void UIToggle::setOnClick(std::function<void()> callback)
{
    this->onClick = callback;
}

void UIToggle::setOffset(glm::vec2 offset)
{
    this->offset = offset;
}

void UIToggle::setAlpha(float alpha)
{
    this->alpha = alpha;
}