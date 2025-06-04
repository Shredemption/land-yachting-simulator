#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <glm/glm.hpp>

#include <string>
#include <array>
#include <functional>

struct ButtonData
{
    std::string text;
    glm::vec2 pos;
    glm::vec2 size;
    float scale;
    std::function<void()> callback;
};

class UIButton
{
public:
    UIButton(const glm::vec2 position, const glm::vec2 size, const std::string &text, const float scale = 1.0f,
             const glm::vec3 baseColor = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec3 hoverColor = glm::vec3(0.8f, 0.8f, 0.8f))
        : pos(position), size(size), onClick(nullptr), text(text), scale(scale), baseColor(baseColor), hoverColor(hoverColor) {};

    bool isHovered(const float mouseX, const float mouseY);
    void checkClicked(const float mouseX, const float mouseY, const bool mousePressed);
    void setOnClick(std::function<void()> callback);

    glm::vec2 pos;
    glm::vec2 size;

    std::string text;
    float scale;
    glm::vec3 baseColor;
    glm::vec3 hoverColor;

    std::function<void()> onClick;
};

#endif