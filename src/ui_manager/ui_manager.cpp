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
        btn->text = "Test Menu";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineState(EngineState::TestMenu); };
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

std::shared_ptr<Widget> buildTestMenu()
{
    auto root = std::make_shared<Widget>();

    float x = 0.025f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Yacht Test";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineScene("test-yacht"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Load Rigid Body Test";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.3f, 0.05f);
        btn->onClick = []()
        { UIManager::queueEngineScene("test-rigid"); };
        btn->index = index++;
        root->AddChild(btn);
    }
    {
        auto btn = std::make_shared<Button>();
        btn->text = "Back";
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

void buildGraphicsPage(std::shared_ptr<Widget> &root)
{
    float x = 0.25f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Fullscreen";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Graphics;
        tgl->linkedVariable = &SettingsManager::settings.video.fullscreen;
        tgl->trueLabel = SettingsManager::settingsMeta.video.fullscreen.trueLabel;
        tgl->falseLabel = SettingsManager::settingsMeta.video.fullscreen.falseLabel;
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
        tgl->trueLabel = SettingsManager::settingsMeta.video.vSync.trueLabel;
        tgl->falseLabel = SettingsManager::settingsMeta.video.vSync.falseLabel;
        tgl->onChange = []()
        { glfwSwapInterval(SettingsManager::settings.video.vSync ? 1 : 0); };
        tgl->index = index++;
        root->AddChild(tgl);
    }
    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "FOV";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Graphics;
        sldr->linkedFloat = &SettingsManager::settings.video.fov;
        sldr->lowerLim = SettingsManager::settingsMeta.video.fov.min;
        sldr->upperLim = SettingsManager::settingsMeta.video.fov.max;
        sldr->stepSize = SettingsManager::settingsMeta.video.fov.stepSize;
        sldr->index = index++;
        root->AddChild(sldr);
    }
    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "View Distance";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Graphics;
        sldr->linkedFloat = &SettingsManager::settings.video.lodDistance;
        sldr->lowerLim = SettingsManager::settingsMeta.video.lodDistance.min;
        sldr->upperLim = SettingsManager::settingsMeta.video.lodDistance.max;
        sldr->stepSize = SettingsManager::settingsMeta.video.lodDistance.stepSize;
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
        sldr->lowerLim = SettingsManager::settingsMeta.video.waterFrameRate.min;
        sldr->upperLim = SettingsManager::settingsMeta.video.waterFrameRate.max;
        sldr->stepSize = SettingsManager::settingsMeta.video.waterFrameRate.stepSize;
        sldr->index = index++;
        root->AddChild(sldr);
    }
}

void buildInputPage(std::shared_ptr<Widget> &root)
{
    float x = 0.25f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "Mouse Sensitivity";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Input;
        sldr->linkedFloat = &SettingsManager::settings.input.mouseSensitivity;
        sldr->lowerLim = SettingsManager::settingsMeta.input.mouseSensitivity.min;
        sldr->upperLim = SettingsManager::settingsMeta.input.mouseSensitivity.max;
        sldr->stepSize = SettingsManager::settingsMeta.input.mouseSensitivity.stepSize;
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
        sldr->lowerLim = SettingsManager::settingsMeta.input.controllerCamSensitivity.min;
        sldr->upperLim = SettingsManager::settingsMeta.input.controllerCamSensitivity.max;
        sldr->stepSize = SettingsManager::settingsMeta.input.controllerCamSensitivity.stepSize;
        sldr->decimals = 2;
        sldr->index = index++;
        root->AddChild(sldr);
    }
}

void buildPhysicsPage(std::shared_ptr<Widget> &root)
{
    float x = 0.25f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto sldr = std::make_shared<Slider>();
        sldr->text = "Tickrate";
        sldr->pos = glm::vec2(x, y + yStep * steps++);
        sldr->size = glm::vec2(0.0f, 0.05f);
        sldr->shownOnPage = SettingsPage::Physics;
        sldr->linkedFloat = &SettingsManager::settings.physics.tickRate;
        sldr->lowerLim = SettingsManager::settingsMeta.physics.tickRate.min;
        sldr->upperLim = SettingsManager::settingsMeta.physics.tickRate.max;
        sldr->stepSize = SettingsManager::settingsMeta.physics.tickRate.stepSize;
        sldr->index = index++;
        root->AddChild(sldr);
    }
}

void buildDebugPage(std::shared_ptr<Widget> &root)
{
    float x = 0.25f;
    float y = 0.15f;
    float yStep = 0.05f;
    int steps = 0;
    int index = 0;

    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Wireframe";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Debug;
        tgl->linkedVariable = &SettingsManager::settings.debug.wireframeMode;
        tgl->trueLabel = SettingsManager::settingsMeta.debug.wireframeMode.trueLabel;
        tgl->falseLabel = SettingsManager::settingsMeta.debug.wireframeMode.falseLabel;
        tgl->index = index++;
        root->AddChild(tgl);
    }
    {
        auto slct = std::make_shared<Selector>();
        slct->text = "Overlay";
        slct->pos = glm::vec2(x, y + yStep * steps++);
        slct->size = glm::vec2(0.0f, 0.05f);
        slct->shownOnPage = SettingsPage::Debug;
        slct->labels = SettingsManager::settingsMeta.debug.debugOverlay.labels;
        slct->currentIndex = static_cast<int>(SettingsManager::settings.debug.debugOverlay);
        slct->onChange = [slct]()
        { SettingsManager::settings.debug.debugOverlay = static_cast<debugOverlay>(slct->currentIndex); };
        slct->index = index++;
        root->AddChild(slct);
    }
    {
        auto tgl = std::make_shared<Toggle>();
        tgl->text = "Hitboxes";
        tgl->pos = glm::vec2(x, y + yStep * steps++);
        tgl->size = glm::vec2(0.0f, 0.05f);
        tgl->shownOnPage = SettingsPage::Debug;
        tgl->linkedVariable = &SettingsManager::settings.debug.showHitboxes;
        tgl->trueLabel = SettingsManager::settingsMeta.debug.showHitboxes.trueLabel;
        tgl->falseLabel = SettingsManager::settingsMeta.debug.showHitboxes.falseLabel;
        tgl->index = index++;
        root->AddChild(tgl);
    }
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
        btn->text = "Physics";
        btn->pos = glm::vec2(x, y + yStep * steps++);
        btn->size = glm::vec2(0.2f, 0.05f);
        btn->activePage = SettingsPage::Physics;
        btn->shownOnPage = SettingsPage::Start;
        btn->onClick = []()
        {
            UIManager::queueSettingsPage(SettingsPage::Physics);
            UIManager::selected = 0;
            UIManager::countOptions(SettingsPage::Physics);
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

    buildGraphicsPage(root);
    buildInputPage(root);
    buildPhysicsPage(root);
    buildDebugPage(root);

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
    case EngineState::TestMenu:
        activeWidgets = buildTestMenu();
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

    if ((current == EngineState::Title || current == EngineState::TestMenu) && (state == EngineState::Title || state == EngineState::TestMenu))
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