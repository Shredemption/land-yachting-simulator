#pragma once

#include <glm/glm.hpp>

#include <string>
#include <array>
#include <functional>

enum class InputType;

class UIToggle
{
public:
    UIToggle(const glm::vec2 position, const glm::vec2 size, const std::string &text, const float scale,
             const glm::vec3 baseColor, const glm::vec3 hoverColor, const glm::vec3 activeColor)
        : pos(position), size(size), toggleVariable(nullptr), text(text), scale(scale), baseColor(baseColor), hoverColor(hoverColor), activeColor(activeColor) {};

    bool isHovered(const float mouseX, const float mouseY);
    void draw(bool selected, bool active, InputType inputType, float mouseX, float mouseY);

    void checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
    {
        if (mousePressed && isHovered(mouseX, mouseY))
            execute();
    };

    void toggle() { *toggleVariable = !(*toggleVariable); };
    void execute()
    {
        toggle();
        if (onClick)
            onClick();
    };

    void setOnClick(std::function<void()> callback) { this->onClick = callback; };
    void setOffset(glm::vec2 offset) { this->offset = offset; };
    void setAlpha(float alpha) { this->alpha = alpha; };

    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 offset = {100.0f, 100.0f};

    std::string text;
    float scale;
    glm::vec3 baseColor;
    glm::vec3 hoverColor;
    glm::vec3 activeColor;
    float alpha = 1.0f;

    bool *toggleVariable;
    std::function<void()> onClick;
};