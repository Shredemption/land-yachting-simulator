#ifndef UI_TOGGLE_H
#define UI_TOGGLE_H

#include <glm/glm.hpp>

#include <string>
#include <array>
#include <functional>

enum class InputType;

class UIToggle
{
public:
    UIToggle(const glm::vec2 position, const glm::vec2 size, const std::string &text, const float scale,
             const glm::vec3 baseColor, const glm::vec3 hoverColor)
        : pos(position), size(size), toggleVariable(nullptr), text(text), scale(scale), baseColor(baseColor), hoverColor(hoverColor) {};

    bool isHovered(const float mouseX, const float mouseY);
    void draw(bool selected, InputType inputType, float mouseX, float mouseY);

    void checkClicked(const float mouseX, const float mouseY, const bool mousePressed)
    {
        if (mousePressed && isHovered(mouseX, mouseY))
            if (toggleVariable)
                *toggleVariable = !(*toggleVariable);
    };

    void setOffset(glm::vec2 offset) { this->offset = offset; };
    void setAlpha(float alpha) { this->alpha = alpha; };

    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 offset = {100.0f, 100.0f};

    std::string text;
    float scale;
    glm::vec3 baseColor;
    glm::vec3 hoverColor;
    float alpha = 1.0f;

    bool *toggleVariable;
};

#endif