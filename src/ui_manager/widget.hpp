#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <vector>
#include <memory>

#include "scene_manager/scene_manager_defs.h"

class Widget
{
public:
    glm::vec2 pos = {0, 0};
    glm::vec2 size = {0, 0};

    std::string text;
    float scale = 0.6f;
    float alpha = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 hoverColor = glm::vec3(0.9f, 0.5f, 0.5f);

    SettingsPage linkedPage = SettingsPage::None;

    virtual ~Widget() = default;

    virtual void Render();
    virtual void Update();

    std::vector<std::shared_ptr<Widget>> children;
    Widget *parent = nullptr;

    void AddChild(std::shared_ptr<Widget> child);
};

class Button : public Widget
{
public:
    std::function<void()> onClick;

    void Render() override;
    void Update() override;
    void Execute();

private:
    bool hover = false;
    bool active = false;

    glm::vec3 activeColor = glm::vec3(0.8f, 0.2f, 0.2f);
};

class Toggle : public Widget
{
public:
    bool *linkedVariable;
    std::function<void()> onChange;

    std::string trueLabel;
    std::string falseLabel;

    float leftOffset = 0.2f;
    float labelOffset = 0.4f;
    float rightOffset = 0.6f;

    void Render() override;
    void Update() override;
    void Execute(bool toRight = true);

private:
    bool hoverLeft = false;
    bool hoverRight = false;
    bool hidden = true;
};

class Selector : public Widget
{
public:
    std::vector<std::string> labels;
    int currentIndex;

    std::function<void()> onChange;

    float leftOffset = 0.2f;
    float labelOffset = 0.4f;
    float rightOffset = 0.6f;

    void Render() override;
    void Update() override;
    void Execute(bool toRight = true);

private:
    bool hoverLeft = false;
    bool hoverRight = false;
    bool hidden = true;
};