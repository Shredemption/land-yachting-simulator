#pragma once

#include <glm/glm.hpp>

#include <string>
#include <array>
#include <functional>

enum class InputType;

class UISelector
{
public:
    UISelector(const glm::vec2 position, const std::string &text, const float scale,
               const glm::vec3 baseColor, const glm::vec3 hoverColor, const glm::vec3 activeColor)
        : pos(position), text(text), scale(scale), baseColor(baseColor), hoverColor(hoverColor), activeColor(activeColor) {};

    void draw(bool selected, InputType inputType, float mouseX, float mouseY);
    bool isInside(const float mouseX, const float mouseY, glm::vec2 pos, glm::vec2 size);

    int currentIndex;
    int prevIndex;
    void updateValue(bool toRight = true);

    std::vector<std::string> optionLabels;
    void setOptionLabels(std::vector<std::string> labels);

    std::function<void(UISelector &)> writeCallback;
    void setOnWrite(std::function<void(UISelector &)> callback);
    std::function<void(UISelector &)> readCallback;
    void setOnRead(std::function<void(UISelector &)> callback);

    glm::vec2 pos;
    glm::vec2 offset = {100.0f, 100.0f};
    void setOffset(glm::vec2 offset);

    std::string text;
    float scale;
    glm::vec3 baseColor;
    glm::vec3 hoverColor;
    glm::vec3 activeColor;
    float alpha = 1.0f;
    void setAlpha(float alpha);

    float labelOffset = 0.4f;
    float arrowSpacing = 0.15f;
    bool hoverLeft = false;
    bool hoverRight = false;
    void checkClicked(const float mouseX, const float mouseY, const bool mousePressed);

    float animTime = 0.0f;
    float animSpeed = 10.0f;
    float slideDistance = 0.1f;
    bool animating = false;
    bool animDirection = true;
    void startAnimation(bool toRight);
};