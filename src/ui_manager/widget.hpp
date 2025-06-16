#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>
#include <vector>
#include <memory>

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

private:
    bool hover;
};