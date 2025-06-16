#include "ui_manager/ui_manager.hpp"

#include "pch.h"

#include "ui_manager/widget.hpp"

std::shared_ptr<Widget> activeWidgets;

std::shared_ptr<Widget> buildTitle()
{
    auto root = std::make_shared<Widget>();

    float x = 0.025f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;

    {
        auto realisticBtn = std::make_shared<Button>();
        realisticBtn->text = "Load Realistic Scene";
        realisticBtn->pos = glm::vec2(x, y + yStep * steps++);
        realisticBtn->size = glm::vec2(0.3f, 0.05f);
        realisticBtn->onClick = []()
        { SceneManager::switchEngineStateScene("realistic"); };
        root->AddChild(realisticBtn);
    }
    {
        auto cartoonButton = std::make_shared<Button>();
        cartoonButton->text = "Load Cartoon Scene";
        cartoonButton->pos = glm::vec2(x, y + yStep * steps++);
        cartoonButton->size = glm::vec2(0.3f, 0.05f);
        cartoonButton->onClick = []()
        { SceneManager::switchEngineStateScene("cartoon"); };
        root->AddChild(cartoonButton);
    }
    {
        auto testBtn = std::make_shared<Button>();
        testBtn->text = "Load Test Scene";
        testBtn->pos = glm::vec2(x, y + yStep * steps++);
        testBtn->size = glm::vec2(0.3f, 0.05f);
        testBtn->onClick = []()
        { SceneManager::switchEngineStateScene("test"); };
        root->AddChild(testBtn);
    }
    {
        auto settingsBtn = std::make_shared<Button>();
        settingsBtn->text = "Settings";
        settingsBtn->pos = glm::vec2(x, y + yStep * steps++);
        settingsBtn->size = glm::vec2(0.3f, 0.05f);
        settingsBtn->onClick = []()
        { SceneManager::switchEngineState(EngineState::TitleSettings); };
        root->AddChild(settingsBtn);
    }
    {
        auto quitBtn = std::make_shared<Button>();
        quitBtn->text = "Quit";
        quitBtn->pos = glm::vec2(x, y + yStep * steps++);
        quitBtn->size = glm::vec2(0.3f, 0.05f);
        quitBtn->onClick = []()
        { SceneManager::switchEngineState(EngineState::None); };
        root->AddChild(quitBtn);
    }

    return root;
}

void UIManager::load(EngineState state)
{
    activeWidgets.reset();

    switch (state)
    {
    case EngineState::Title:
        activeWidgets = buildTitle();
        break;
    }
}

void UIManager::update()
{
    if (activeWidgets)
        activeWidgets->Update();
}

void UIManager::render()
{
    if (activeWidgets)
        activeWidgets->Render();
}