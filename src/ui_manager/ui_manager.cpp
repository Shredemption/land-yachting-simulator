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
    int index = 0;

    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Realistic Scene";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineStateScene("realistic"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Cartoon Scene";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineStateScene("cartoon"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Test Scene";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineStateScene("test"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Settings";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineState(EngineState::TitleSettings); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Quit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineState(EngineState::None); };
        btn->index = index++;
        root->AddChild(btn);
    }

    UIManager::options = index;

    return root;
}

std::shared_ptr<Widget> buildPause()
{
    auto root = std::make_shared<Widget>();

    float x = 0.025f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto btn = std::make_shared<Button>();
        btn->text = "Resume";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineState(EngineState::Running); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Settings";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineState(EngineState::Settings); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Exit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { SceneManager::switchEngineState(EngineState::Title); };
        btn->index = index++;
        root->AddChild(btn);
    }

    UIManager::options = index;

    return root;
}

std::shared_ptr<Widget> buildSettings(EngineState state)
{
    auto root = std::make_shared<Widget>();

    float x = 0.025f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto btn = std::make_shared<Button>();
        btn->text = "Graphics";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->linkedPage = SettingsPage::Graphics;
        btn->onClick = []()
        { SceneManager::settingsPage = SettingsPage::Graphics; };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Debug";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->linkedPage = SettingsPage::Debug;
        btn->onClick = []()
        { SceneManager::settingsPage = SettingsPage::Debug; };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Exit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);

        if (state == EngineState::Settings)
            btn->onClick = []()
            {
                SceneManager::settingsPage = SettingsPage::None;
                SceneManager::switchEngineState(EngineState::Pause);
            };
        else
            btn->onClick = []()
            {
                SceneManager::settingsPage = SettingsPage::None;
                SceneManager::switchEngineState(EngineState::Title);
            };

        btn->index = index++;
        root->AddChild(btn);
    }

    // Graphics Page
    x = 0.3f;
    y = 0.15f;
    yStep = 0.05f;
    steps = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Fullscreen";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.3f, 0.05f);
        tgl->linkedPage = SettingsPage::Graphics;
        tgl->linkedVariable = &SettingsManager::settings.video.fullscreen;
        tgl->trueLabel = "Borderless";
        tgl->falseLabel = "Off";
        tgl->onChange = []()
        { WindowManager::setFullscreenState(); };
        root->AddChild(tgl);
    }
    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "VSync";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.3f, 0.05f);
        tgl->linkedPage = SettingsPage::Graphics;
        tgl->linkedVariable = &SettingsManager::settings.video.vSync;
        tgl->trueLabel = "On";
        tgl->falseLabel = "Off";
        tgl->onChange = []()
        { glfwSwapInterval(SettingsManager::settings.video.vSync ? 1 : 0); };
        root->AddChild(tgl);
    }

    // Debug Page
    x = 0.3f;
    y = 0.15f;
    yStep = 0.05f;
    steps = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Wireframe";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.3f, 0.05f);
        tgl->linkedPage = SettingsPage::Debug;
        tgl->linkedVariable = &SettingsManager::settings.debug.wireframeMode;
        tgl->trueLabel = "On";
        tgl->falseLabel = "Off";
        root->AddChild(tgl);
    }
    {
        auto slt = std::make_shared<Selector>();
        slt->text = "Overlay";
        slt->pos = glm::vec2(x, y + yStep * steps++);
        slt->size = glm::vec2(0.3f, 0.05f);
        slt->linkedPage = SettingsPage::Debug;
        slt->labels = {"Off", "FPS", "Physics"};
        slt->currentIndex = static_cast<int>(SettingsManager::settings.debug.debugOverlay);
        slt->onChange = [slt]()
        { SettingsManager::settings.debug.debugOverlay = static_cast<debugOverlay>(slt->currentIndex); };
        root->AddChild(slt);
    }

    UIManager::options = index;

    return root;
}

void UIManager::load(EngineState state)
{
    activeWidgets.reset();

    selected = 0;

    switch (state)
    {
    case EngineState::Title:
        activeWidgets = buildTitle();
        break;
    case EngineState::Pause:
        activeWidgets = buildPause();
        break;
    case EngineState::Settings:
    case EngineState::TitleSettings:
        activeWidgets = buildSettings(state);
        break;
    }
}

void UIManager::update()
{
    if (activeWidgets)
        activeWidgets->Update();

    trigger = false;
}

void UIManager::render()
{
    if (activeWidgets)
        activeWidgets->Render();
}