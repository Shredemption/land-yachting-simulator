#pragma once

#include <glm/glm.hpp>

#include <string>
#include <array>
#include <functional>

enum class InputType;

class UIToggle
{
public:
    UIToggle(const glm::vec2 position, const std::string &text, const float scale,
             const glm::vec3 baseColor, const glm::vec3 hoverColor, const glm::vec3 activeColor)
        : pos(position), toggleVariable(nullptr), text(text), scale(scale), baseColor(baseColor), hoverColor(hoverColor), activeColor(activeColor) {};

    void draw(bool selected, InputType inputType, float mouseX, float mouseY);
    bool isInside(const float mouseX, const float mouseY, glm::vec2 pos, glm::vec2 size);

    void checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
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

    void toggle()
    {
        if (!toggleVariable)
            return;

        startAnimation(!(toggleVariable));
        *toggleVariable = !(*toggleVariable);
    }

    void execute()
    {
        toggle();
        if (onClick)
            onClick();
    }

    void setOnClick(std::function<void()> callback) { this->onClick = callback; };
    void setOffset(glm::vec2 offset) { this->offset = offset; };
    void setAlpha(float alpha) { this->alpha = alpha; };
    void setTextOptions(const std::string &falseLabel, const std::string &trueLabel)
    {
        falseText = falseLabel;
        trueText = trueLabel;
    }

    void startAnimation(bool toTrue)
    {
        animDirection = toTrue;
        animTime = 0.0f;
        animating = true;
    }

    glm::vec2 pos;
    glm::vec2 offset = {100.0f, 100.0f};

    std::string text;
    float scale;
    glm::vec3 baseColor;
    glm::vec3 hoverColor;
    glm::vec3 activeColor;
    float alpha = 1.0f;

    std::string falseText = "Off";
    std::string trueText = "On";
    float labelOffset = 0.4f;
    float arrowSpacing = 0.15f;
    bool hoverLeft = false;
    bool hoverRight = false;

    float animTime = 0.0f;
    float animSpeed = 10.0f;
    float slideDistance = 0.1f;
    bool animating = false;
    bool animDirection = true;

    bool *toggleVariable;
    std::function<void()> onClick;
};