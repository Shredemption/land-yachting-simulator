#include "ui_manager/ui_manager.hpp"

#include "pch.h"

void UIManager::update()
{
    if (InputManager::inputType == InputType::Mouse)
    {
        for (auto &button : buttons)
        {
            button.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }

        for (auto &button : buttonsSide)
        {
            button.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
        for (auto &toggle : togglesSide)
        {
            toggle.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
        for (auto &selector : selectorsSide)
        {
            selector.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
    }

    draw();
}

void UIManager::load(const EngineState &state)
{
    selectedMain = 0;

    buttons.clear();
    uiElements.clear();

    std::vector<UIElementData> elements = {};

    float scale = 0.5;
    glm::vec3 baseColor = defaultBaseColor;
    glm::vec3 hoverColor = defaultHoverColor;
    glm::vec3 activeColor = defaultActiveColor;

    glm::vec2 startPos(0.03, 0.20);
    glm::vec2 stepPos(0.0, 0.05);

    glm::vec2 size(0.2, scale / 10);

    switch (state)
    {
    case EngineState::Title:

        size = {0.3, scale / 10};

        elements = {
            {
                UIElementType::Button,
                "Load Realistic Scene",
                []
                { SceneManager::switchEngineStateScene("realistic"); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Load Cartoon Scene",
                []
                { SceneManager::switchEngineStateScene("cartoon"); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Load Test Scene",
                []
                { SceneManager::switchEngineStateScene("test"); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::TitleSettings); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Quit",
                []
                { SceneManager::switchEngineState(EngineState::None); },
                nullptr,
            },
        };

        break;

    case EngineState::Pause:

        elements = {
            {
                UIElementType::Button,
                "Resume",
                []
                { SceneManager::switchEngineState(EngineState::Running); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Restart",
                []
                {
                    Camera::reset();
                    PhysicsUtil::setup();
                    SceneManager::runOneFrame();
                    SceneManager::switchEngineState(EngineState::Running);
                },
                nullptr,
            },
            {
                UIElementType::Button,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::Settings); },
                nullptr,
            },
            {
                UIElementType::Button,
                "Exit to Menu",
                []
                { SceneManager::switchEngineState(EngineState::Title); },
                nullptr,
            },
        };

        break;

    case EngineState::Settings:
    case EngineState::TitleSettings:

        elements = {
            {
                UIElementType::Button,
                "Graphics",
                []
                {
                    SceneManager::switchSettingsPage(SettingsPage::Graphics);
                    inputState = UIInputState::Side;
                },
                nullptr,
                SettingsPage::Graphics,
            },
            {
                UIElementType::Button,
                "Debug",
                []
                {
                    SceneManager::switchSettingsPage(SettingsPage::Debug);
                    inputState = UIInputState::Side;
                },
                nullptr,
                SettingsPage::Debug,
            },
            {UIElementType::Button,
             "Back",
             []
             {
                 if (SceneManager::engineState == EngineState::Settings)
                 {
                     SceneManager::switchEngineState(EngineState::Pause);
                 }
                 else
                 {
                     SceneManager::switchEngineState(EngineState::Title);
                 }

                 SceneManager::switchSettingsPage(SettingsPage::Start);
             }},
        };
        break;
    }

    std::vector<UIToggle> dummyToggles;
    std::vector<UISelector> dummySelectors;

    loadElements(elements, startPos, stepPos, size,
                 scale, baseColor, hoverColor, activeColor,
                 uiElements, buttons, dummyToggles, dummySelectors);
    rebuildTotalElements();
}

void UIManager::loadSide(const SettingsPage &page)
{
    selectedSide = (InputManager::inputType != InputType::Mouse) ? 0 : -1;

    buttonsSide.clear();
    togglesSide.clear();
    uiElementsSide.clear();

    std::vector<UIElementData> elementsSide = {};

    float scaleSide = 0.5;
    glm::vec3 baseColorSide = defaultBaseColor;
    glm::vec3 hoverColorSide = defaultHoverColor;
    glm::vec3 activeColorSide = defaultActiveColor;

    glm::vec2 startPosSide(0.25, 0.20);
    glm::vec2 stepPosSide(0.0, 0.05);

    glm::vec2 sizeSide(0.3, scaleSide / 10);

    switch (page)
    {
    case SettingsPage::Start:

        elementsSide = {};

        break;

    case SettingsPage::Graphics:

        elementsSide = {
            {
                UIElementType::Toggle,
                "Fullscreen",
                []
                {
                    WindowManager::setFullscreenState();
                    SceneManager::runOneFrame();
                },
                &SettingsManager::settings.video.fullscreen,
                std::nullopt,
                {"Borderless", "Off"},
            },
            {
                UIElementType::Toggle,
                "VSync",
                []
                {
                    glfwSwapInterval(SettingsManager::settings.video.vSync ? 1 : 0);
                },
                &SettingsManager::settings.video.vSync,
                std::nullopt,
                {"On", "Off"},
            },
        };

        break;

    case SettingsPage::Debug:

        elementsSide = {
            {
                UIElementType::Toggle,
                "Wireframe",
                {},
                &SettingsManager::settings.debug.wireframeMode,
                std::nullopt,
                {"On", "Off"},
            },
            {
                UIElementType::Selector,
                "Debug Overlay",
                {},
                nullptr,
                std::nullopt,
                {"Off", "FPS", "Physics"},
                [](UISelector &sel)
                { sel.currentIndex = static_cast<int>(SettingsManager::settings.debug.debugOverlay); },
                [](UISelector &sel)
                { SettingsManager::settings.debug.debugOverlay = static_cast<debugOverlay>(sel.currentIndex); },
            },
        };

        break;
    }

    loadElements(elementsSide, startPosSide, stepPosSide, sizeSide,
                 scaleSide, baseColorSide, hoverColorSide, activeColorSide,
                 uiElementsSide, buttonsSide, togglesSide, selectorsSide);
    rebuildTotalElements();
}

void UIManager::loadElements(std::vector<UIElementData> elementData, glm::vec2 startPos, glm::vec2 stepPos, glm::vec2 size, float scale, glm::vec3 baseCol, glm::vec3 hoverCol, glm::vec3 activeCol,
                             std::vector<UIElement> &UIelements, std::vector<UIButton> &UIbuttons, std::vector<UIToggle> &UItoggles, std::vector<UISelector> &UIselectors)
{
    for (int i = 0; i < elementData.size(); i++)
    {
        UIElementData element = elementData[i];

        switch (element.type)
        {
        case UIElementType::Button:
            UIbuttons.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseCol, hoverCol, activeCol);
            UIbuttons.back().setOnClick(element.callback);
            UIbuttons.back().linkedPage = element.linkedPage;
            break;

        case UIElementType::Toggle:
            UItoggles.emplace_back(startPos + (float(i) * stepPos), element.text, scale, baseCol, hoverCol, activeCol);
            UItoggles.back().setOnClick(element.callback);
            UItoggles.back().toggleVariable = element.toggleVariable;
            UItoggles.back().setTextOptions(element.optionLabels[1], element.optionLabels[0]);
            break;
        case UIElementType::Selector:
            UIselectors.emplace_back(startPos + (float(i) * stepPos), element.text, scale, baseCol, hoverCol, activeCol);
            UIselectors.back().setOnWrite(element.writeCallback);
            UIselectors.back().setOnRead(element.readCallback);
            UIselectors.back().setOptionLabels(element.optionLabels);
            UIselectors.back().readCallback(UIselectors.back());
            break;
        }
    }

    int buttonIndex = 0;
    int toggleIndex = 0;
    int selectorIndex = 0;
    for (const auto &element : elementData)
    {
        switch (element.type)
        {
        case UIElementType::Button:
            UIelements.push_back(&UIbuttons[buttonIndex++]);
            break;
        case UIElementType::Toggle:
            UIelements.push_back(&UItoggles[toggleIndex++]);
            break;
        case UIElementType::Selector:
            UIelements.push_back(&UIselectors[selectorIndex++]);
        }
    }
}

void UIManager::rebuildTotalElements()
{
    uiElementsTotal.clear();

    int mainIdx = 0;
    for (auto &element : uiElements)
        uiElementsTotal.push_back({&element, UIInputState::Main, mainIdx++});
    int sideIdx = 0;
    for (auto &element : uiElementsSide)
        uiElementsTotal.push_back({&element, UIInputState::Side, sideIdx++});
}

UIElement *UIManager::getSelectedElement()
{
    int selectedIndex = -1;
    switch (inputState)
    {
    case UIInputState::Main:
        selectedIndex = selectedMain;
        break;
    case UIInputState::Side:
        selectedIndex = selectedSide;
        break;
    }

    if (selectedIndex == -1)
        return nullptr;

    int groupIndex = 0;
    for (auto &elementRef : uiElementsTotal)
    {
        if (elementRef.linkedState != inputState)
            continue;

        if (groupIndex == selectedIndex)
            return elementRef.element;

        ++groupIndex;
    }

    return nullptr;
}

void UIManager::draw()
{
    for (const auto &ref : uiElementsTotal)
    {
        if (!ref.element)
            continue;

        std::visit([&](auto *element)
                   {
                       if (!element)
                           return;

                       bool active = false;
                       if constexpr (std::is_same_v<std::decay_t<decltype(*element)>, UIButton>)
                       {
                           if (element->linkedPage == SceneManager::settingsPage)
                               active = true;

                           bool isSelected = false;
                           if (ref.linkedState == UIInputState::Main && inputState == UIInputState::Main)
                               isSelected = ref.localIndex == selectedMain;
                           else if (ref.linkedState == UIInputState::Side && inputState == UIInputState::Side)
                               isSelected = ref.localIndex == selectedSide;

                           element->draw(isSelected, active, InputManager::inputType, InputManager::mousePosX, InputManager::mousePosY);
                       }

                       if constexpr (std::is_same_v<std::decay_t<decltype(*element)>, UIToggle>)
                       {
                           bool isSelected = false;
                           if (ref.linkedState == UIInputState::Main && inputState == UIInputState::Main)
                               isSelected = ref.localIndex == selectedMain;
                           else if (ref.linkedState == UIInputState::Side && inputState == UIInputState::Side)
                               isSelected = ref.localIndex == selectedSide;

                           element->draw(isSelected, InputManager::inputType, InputManager::mousePosX, InputManager::mousePosY);
                       }

                       if constexpr (std::is_same_v<std::decay_t<decltype(*element)>, UISelector>)
                       {
                           bool isSelected = false;
                           if (ref.linkedState == UIInputState::Main && inputState == UIInputState::Main)
                               isSelected = ref.localIndex == selectedMain;
                           else if (ref.linkedState == UIInputState::Side && inputState == UIInputState::Side)
                               isSelected = ref.localIndex == selectedSide;

                           element->draw(isSelected, InputManager::inputType, InputManager::mousePosX, InputManager::mousePosY);
                       } },
                   *ref.element);
    }
}