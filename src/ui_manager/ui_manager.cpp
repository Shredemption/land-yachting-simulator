#include "ui_manager/ui_manager.hpp"

#include "pch.h"

#include "ui_manager/widget.hpp"
#include "ui_manager/ui_manager_defs.h"

std::shared_ptr<Widget> activeWidgets;

std::optional<EngineState> queuedState;
std::optional<std::string> queuedScene;
std::optional<SettingsPage> queuedPage;

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
        { UIManager::queueEngineScene("realistic"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Cartoon Scene";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineScene("cartoon"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Test Scene";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineScene("test-yacht"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Settings";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineState(EngineState::TitleSettings); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Quit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineState(EngineState::None); };
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
        { UIManager::queueEngineState(EngineState::Running); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Settings";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineState(EngineState::Settings); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Exit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineState(EngineState::Title); };
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
        btn->activePage = SettingsPage::Graphics;
        btn->shownOnPage = SettingsPage::Start;
        btn->onClick = []()
        {
            UIManager::queueSettingsPage(SettingsPage::Graphics);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Graphics);
        };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Input";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->activePage = SettingsPage::Input;
        btn->shownOnPage = SettingsPage::Start;
        btn->onClick = []()
        {
            UIManager::queueSettingsPage(SettingsPage::Input);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Input);
        };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Debug";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->activePage = SettingsPage::Debug;
        btn->shownOnPage = SettingsPage::Start;
        btn->onClick = []()
        {
            UIManager::queueSettingsPage(SettingsPage::Debug);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Debug);
        };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Exit";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->shownOnPage = SettingsPage::Start;
        if (state == EngineState::Settings)
            btn->onClick = []()
            {
                UIManager::queueSettingsPage(SettingsPage::None);
                UIManager::queueEngineState(EngineState::Pause);
            };
        else
            btn->onClick = []()
            {
                UIManager::queueSettingsPage(SettingsPage::None);
                UIManager::queueEngineState(EngineState::Title);
            };
        btn->index = index++;
        root->AddChild(btn);
    }

    UIManager::options = index;

    // Graphics Page
    x = 0.25f;
    y = 0.15f;
    yStep = 0.05f;
    steps = 0;
    index = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Fullscreen";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Graphics;
        tgl->linkedVariable = &SettingsManager::settings.video.fullscreen;
        tgl->trueLabel = "Borderless";
        tgl->falseLabel = "Off";
        tgl->onChange = []()
        { WindowManager::setFullscreenState(); };
        tgl->index = index++;
        root->AddChild(tgl);
    }
    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "VSync";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Graphics;
        tgl->linkedVariable = &SettingsManager::settings.video.vSync;
        tgl->trueLabel = "On";
        tgl->falseLabel = "Off";
        tgl->onChange = []()
        { glfwSwapInterval(SettingsManager::settings.video.vSync ? 1 : 0); };
        tgl->index = index++;
        root->AddChild(tgl);
    }
    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "View Distance";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Graphics;
        sldr->linkedFloat = &SettingsManager::settings.video.lodDistance;
        sldr->lowerLim = 10.0f;
        sldr->upperLim = 100.0f;
        sldr->stepSize = 10.0f;
        sldr->index = index++;
        root->AddChild(sldr);
    }
    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "Water Framerate";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Graphics;
        sldr->linkedFloat = &SettingsManager::settings.video.waterFrameRate;
        sldr->lowerLim = 10.0f;
        sldr->upperLim = 120.0f;
        sldr->stepSize = 1.0f;
        sldr->index = index++;
        root->AddChild(sldr);
    }

    // Input Page
    x = 0.25f;
    y = 0.15f;
    yStep = 0.05f;
    steps = 0;
    index = 0;

    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "Mouse Sensitivity";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Input;
        sldr->linkedFloat = &SettingsManager::settings.input.mouseSensitivity;
        sldr->lowerLim = 2.0f;
        sldr->upperLim = 8.0f;
        sldr->stepSize = 0.1f;
        sldr->decimals = 2;
        sldr->index = index++;
        root->AddChild(sldr);
    }
    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "Stick Sensitivity";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Input;
        sldr->linkedFloat = &SettingsManager::settings.input.controllerCamSensitivity;
        sldr->lowerLim = 2.0f;
        sldr->upperLim = 8.0f;
        sldr->stepSize = 0.1f;
        sldr->decimals = 2;
        sldr->index = index++;
        root->AddChild(sldr);
    }

    // Debug Page
    x = 0.25f;
    y = 0.15f;
    yStep = 0.05f;
    steps = 0;
    index = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Wireframe";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Debug;
        tgl->linkedVariable = &SettingsManager::settings.debug.wireframeMode;
        tgl->trueLabel = "On";
        tgl->falseLabel = "Off";
        tgl->index = index++;
        root->AddChild(tgl);
    }
    {
        auto slct = std::make_shared<Selector>();
        slct->text = "Overlay";
        slct->pos = glm::vec2(x, y + yStep * steps++);
        slct->size = glm::vec2(0.0f, 0.05f);
        slct->shownOnPage = SettingsPage::Debug;
        slct->labels = {"Off", "FPS", "Physics"};
        slct->currentIndex = static_cast<int>(SettingsManager::settings.debug.debugOverlay);
        slct->onChange = [slct]()
        { SettingsManager::settings.debug.debugOverlay = static_cast<debugOverlay>(slct->currentIndex); };
        slct->index = index++;
        root->AddChild(slct);
    }

    return root;
}

void UIManager::countOptions(SettingsPage page)
{
    UIManager::options = 0;

    for (auto widget : activeWidgets->children)
    {
        if (widget->shownOnPage == page)
            UIManager::options++;
    }
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
    if (fade <= fadeTime)
    {
        if (queuedState.has_value())
        {
            fade -= TimeManager::deltaTime;
            if (fade <= 0.0f)
            {
                SceneManager::switchEngineState(queuedState.value());
                queuedState.reset();
            }
        }
        else if (queuedScene.has_value())
        {
            fade -= TimeManager::deltaTime;
            if (fade <= 0.0f)
            {
                SceneManager::switchEngineStateScene(queuedScene.value());
                queuedScene.reset();
            }
        }
        else
        {
            fade += TimeManager::deltaTime;
        }

        return;
    }

    if (pageFade <= fadeTime)
    {
        if (queuedPage.has_value())
        {
            pageFade -= TimeManager::deltaTime;
            if (pageFade <= 0.0f)
            {
                SceneManager::settingsPage = queuedPage.value();
                queuedPage.reset();
            }
        }
        else
        {
            pageFade += TimeManager::deltaTime;
        }
    }

    if (activeWidgets)
        activeWidgets->Update();

    trigger = false;
    triggerLeft = false;
    triggerRight = false;
}

void UIManager::render()
{
    if (activeWidgets)
        activeWidgets->Render();
}

void UIManager::queueEngineState(EngineState state)
{
    shouldFadeBackground = true;
    fadeToBlack = false;

    EngineState current = SceneManager::engineState;

    if ((current == EngineState::Title || current == EngineState::TitleSettings) && (state == EngineState::Title || state == EngineState::TitleSettings))
        shouldFadeBackground = false;

    if ((current == EngineState::Pause || current == EngineState::Settings) && (state == EngineState::Pause || state == EngineState::Settings))
        shouldFadeBackground = false;

    if ((current == EngineState::Pause || current == EngineState::Title) && (state == EngineState::Pause || state == EngineState::Title))
        fadeToBlack = true;

    fade = fadeTime;
    queuedState = state;
}

void UIManager::queueEngineScene(std::string scene)
{
    shouldFadeBackground = true;
    fade = fadeTime;
    queuedScene = scene;
}

void UIManager::queueSettingsPage(SettingsPage page)
{
    if (SceneManager::settingsPage == SettingsPage::Start)
        pageFade = 0.0f;
    else
        pageFade = fadeTime;

    queuedPage = page;
}