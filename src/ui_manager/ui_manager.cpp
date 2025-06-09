#include "ui_manager/ui_manager.hpp"
#include "ui_manager/ui_button.h"
#include "ui_manager/ui_toggle.h"

#include "pch.h"

void UIManager::update()
{
    if (InputManager::inputType == InputType::itMouse)
    {
        for (auto &button : buttons)
        {
            button.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
        for (auto &toggle : toggles)
        {
            toggle.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }

        for (auto &button : buttonsSide)
        {
            button.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
        for (auto &toggle : togglesSide)
        {
            toggle.checkClicked(InputManager::mousePosX, InputManager::mousePosY, InputManager::leftMouseButton.pressed());
        }
    }

    draw();
}

void UIManager::load(const EngineState &state)
{
    selectedMain = 0;

    buttons.clear();
    toggles.clear();
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
    case EngineState::esTitle:

        size = {0.3, scale / 10};

        elements = {
            {
                UIElementType::uiButton,
                "Load Realistic Scene",
                []
                { SceneManager::switchEngineStateScene("realistic"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Load Cartoon Scene",
                []
                { SceneManager::switchEngineStateScene("cartoon"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Load Test Scene",
                []
                { SceneManager::switchEngineStateScene("test"); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::esTitleSettings); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Quit",
                []
                { SceneManager::switchEngineState(EngineState::esNone); },
                nullptr,
            },
        };

        break;

    case EngineState::esPause:

        elements = {
            {
                UIElementType::uiButton,
                "Resume",
                []
                { SceneManager::switchEngineState(EngineState::esRunning); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Restart",
                []
                {
                    Camera::reset();
                    PhysicsUtil::setup();
                    SceneManager::runOneFrame();
                    SceneManager::switchEngineState(EngineState::esRunning);
                },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Settings",
                []
                { SceneManager::switchEngineState(EngineState::esSettings); },
                nullptr,
            },
            {
                UIElementType::uiButton,
                "Exit to Menu",
                []
                { SceneManager::switchEngineState(EngineState::esTitle); },
                nullptr,
            },
        };

        break;

    case EngineState::esSettings:
    case EngineState::esTitleSettings:

        elements = {
            {
                UIElementType::uiButton,
                "Graphics",
                []
                {
                    SceneManager::switchSettingsPage(SettingsPage::spGraphics);
                    inputState = UIInputState::uiSide;
                },
                nullptr,
                SettingsPage::spGraphics,
            },
            {
                UIElementType::uiButton,
                "Debug",
                []
                {
                    SceneManager::switchSettingsPage(SettingsPage::spDebug);
                    inputState = UIInputState::uiSide;
                },
                nullptr,
                SettingsPage::spDebug,
            },
            {
                UIElementType::uiButton,
                "Back",
                []
                {
                    if (SceneManager::engineState == EngineState::esSettings)
                    {
                        SceneManager::switchEngineState(EngineState::esPause);
                    }
                    else
                    {
                        SceneManager::switchEngineState(EngineState::esTitle);
                    }

                    SceneManager::switchSettingsPage(SettingsPage::spStart);
                },
                nullptr,
            },
        };
        break;
    }

    for (int i = 0; i < elements.size(); i++)
    {
        UIElementData element = elements[i];

        switch (element.type)
        {
        case UIElementType::uiButton:
            buttons.emplace_back(startPos + (float(i) * stepPos), size, element.text, scale, baseColor, hoverColor, activeColor);
            buttons.back().setOnClick(element.callback);
            buttons.back().linkedPage = element.linkedPage;

            break;

        case UIElementType::uiToggle:
            toggles.emplace_back(startPos + (float(i) * stepPos), element.text, scale, baseColor, hoverColor, activeColor);
            toggles.back().toggleVariable = element.toggleVariable;
            break;
        }
    }

    int buttonIndex = 0;
    int toggleIndex = 0;
    for (const auto &element : elements)
    {
        switch (element.type)
        {
        case UIElementType::uiButton:
            uiElements.push_back(&buttons[buttonIndex++]);
            break;
        case UIElementType::uiToggle:
            uiElements.push_back(&toggles[toggleIndex++]);
            break;
        }
    }

    rebuildTotalElements();
}

void UIManager::loadSide(const SettingsPage &page)
{
    selectedSide = 0;

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
    case SettingsPage::spStart:

        elementsSide = {};

        break;

    case SettingsPage::spGraphics:

        elementsSide = {
            {
                UIElementType::uiToggle,
                "Fullscreen",
                []
                {
                    WindowManager::setFullscreenState();
                    SceneManager::runOneFrame();
                },
                &SettingsManager::settings.video.fullscreen,
            },
            {
                UIElementType::uiToggle,
                "VSync",
                []
                {
                    glfwSwapInterval(SettingsManager::settings.video.vSync ? 1 : 0);
                },
                &SettingsManager::settings.video.vSync,
            },
        };

        break;

    case SettingsPage::spDebug:

        elementsSide = {
            {
                UIElementType::uiToggle,
                "Wireframe",
                {},
                &SettingsManager::settings.debug.wireframeMode,
            },
        };

        break;
    }

    for (int i = 0; i < elementsSide.size(); i++)
    {
        UIElementData element = elementsSide[i];

        switch (element.type)
        {
        case UIElementType::uiButton:
            buttonsSide.emplace_back(startPosSide + (float(i) * stepPosSide), sizeSide, element.text, scaleSide, baseColorSide, hoverColorSide, activeColorSide);
            buttonsSide.back().setOnClick(element.callback);
            buttonsSide.back().linkedPage = element.linkedPage;
            break;

        case UIElementType::uiToggle:
            togglesSide.emplace_back(startPosSide + (float(i) * stepPosSide), element.text, scaleSide, baseColorSide, hoverColorSide, activeColorSide);
            togglesSide.back().setOnClick(element.callback);
            togglesSide.back().toggleVariable = element.toggleVariable;
            break;
        }
    }

    int buttonIndex = 0;
    int toggleIndex = 0;
    for (const auto &element : elementsSide)
    {
        switch (element.type)
        {
        case UIElementType::uiButton:
            uiElementsSide.push_back(&buttonsSide[buttonIndex++]);
            break;
        case UIElementType::uiToggle:
            uiElementsSide.push_back(&togglesSide[toggleIndex++]);
            break;
        }
    }

    rebuildTotalElements();
}

void UIManager::rebuildTotalElements()
{
    uiElementsTotal.clear();

    int mainIdx = 0;
    for (auto &element : uiElements)
        uiElementsTotal.push_back({&element, UIInputState::uiMain, mainIdx++});
    int sideIdx = 0;
    for (auto &element : uiElementsSide)
        uiElementsTotal.push_back({&element, UIInputState::uiSide, sideIdx++});
}

UIElement *UIManager::getSelectedElement()
{
    int selectedIndex = -1;
    switch (inputState)
    {
    case UIInputState::uiMain:
        selectedIndex = selectedMain;
        break;
    case UIInputState::uiSide:
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
            if (!element) return;

            bool active = false;
            if constexpr (std::is_same_v<std::decay_t<decltype(*element)>, UIButton>)
            {
                if (element->linkedPage == SceneManager::settingsPage)
                    active = true;

                bool isSelected = false;
                if (ref.linkedState == UIInputState::uiMain && inputState == UIInputState::uiMain)
                    isSelected = ref.localIndex == selectedMain;
                else if (ref.linkedState == UIInputState::uiSide && inputState == UIInputState::uiSide)
                    isSelected = ref.localIndex == selectedSide;

                element->draw(isSelected, active, InputManager::inputType, InputManager::mousePosX, InputManager::mousePosY);
            } 

            if constexpr (std::is_same_v<std::decay_t<decltype(*element)>, UIToggle>)
                {
                    bool isSelected = false;
                    if (ref.linkedState == UIInputState::uiMain && inputState == UIInputState::uiMain)
                        isSelected = ref.localIndex == selectedMain;
                    else if (ref.linkedState == UIInputState::uiSide && inputState == UIInputState::uiSide)
                        isSelected = ref.localIndex == selectedSide;
                        
                   element->draw(isSelected, InputManager::inputType, InputManager::mousePosX, InputManager::mousePosY);
                } },
                   *ref.element);
    }
}

bool UIButton::isHovered(const float mouseX, const float mouseY)
{
    float xmin = pos.x * WindowManager::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x) * WindowManager::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.005f) * WindowManager::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.005f + size.y) * WindowManager::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIButton::draw(bool selected, bool active, InputType inputType, float mouseX, float mouseY)
{
    glm::vec3 color = baseColor;

    if (inputType == InputType::itMouse)
        color = isHovered(mouseX, mouseY) ? hoverColor : active ? activeColor
                                                                : baseColor;
    else
        color = selected ? hoverColor : active ? activeColor
                                               : baseColor;

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text, x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text, x, y, scale, color, alpha);
    glEnable(GL_DEPTH_TEST);
}

bool UIToggle::isInside(const float mouseX, const float mouseY, glm::vec2 pos, glm::vec2 size)
{
    float xmin = (pos.x - size.x / 2.0f) * WindowManager::screenUIScale * 2560.0f;
    float xmax = (pos.x + size.x / 2.0f) * WindowManager::screenUIScale * 2560.0f;

    float ymin = (pos.y - 0.005f) * WindowManager::screenUIScale * 1440.0f;
    float ymax = (pos.y - 0.005f + size.y) * WindowManager::screenUIScale * 1440.0f;

    return mouseX >= xmin && mouseX <= xmax && mouseY >= ymin && mouseY <= ymax;
}

void UIToggle::draw(bool selected, InputType inputType, float mouseX, float mouseY)
{
    if (animating)
    {
        animTime += animSpeed * TimeManager::deltaTime;
        if (animTime >= 1.0f)
        {
            animTime = 1.0f;
            animating = false;
        }
    }

    const std::string &currentLabel = *toggleVariable ? trueText : falseText;
    const std::string &prevLabel = *toggleVariable ? falseText : trueText;

    float slide = 0.0f;
    if (animating)
    {
        float progress = animTime;
        slide = (animDirection ? (1.0f - progress) : progress) * slideDistance;
    }

    float x = pos.x + offset.x;
    float y = pos.y + offset.y;

    glDisable(GL_DEPTH_TEST);
    Render::renderText(text, x + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha);
    Render::renderText(text, x, y, scale, baseColor, alpha);

    Render::renderText("<", x + labelOffset - arrowSpacing + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
    Render::renderText(">", x + labelOffset + arrowSpacing + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
    Render::renderText("<", x + labelOffset - arrowSpacing, y, scale, (selected || hoverLeft) ? hoverColor : baseColor, alpha, TextAlign::Center);
    Render::renderText(">", x + labelOffset + arrowSpacing, y, scale, (selected || hoverRight) ? hoverColor : baseColor, alpha, TextAlign::Center);

    if (animating)
    {
        float alphaA = std::clamp(1.0f - animTime * animTime, 0.0f, 1.0f);
        float alphaB = std::clamp(animTime * animTime, 0.0f, 1.0f);

        Render::renderText(prevLabel,
                           x + labelOffset + (animDirection ? -slide : slide) + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha * alphaA, TextAlign::Center);
        Render::renderText(currentLabel,
                           x + labelOffset + (animDirection ? slideDistance - slide : slide - slideDistance) + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha * alphaB, TextAlign::Center);

        Render::renderText(prevLabel,
                           x + labelOffset + (animDirection ? -slide : slide), y, scale, baseColor, alpha * alphaA, TextAlign::Center);
        Render::renderText(currentLabel,
                           x + labelOffset + (animDirection ? slideDistance - slide : slide - slideDistance), y, scale, baseColor, alpha * alphaB, TextAlign::Center);
    }
    else
    {
        Render::renderText(currentLabel, x + labelOffset + 0.003f, y + 0.003f, scale, glm::vec3(0, 0, 0), alpha, TextAlign::Center);
        Render::renderText(currentLabel, x + labelOffset, y, scale, baseColor, alpha, TextAlign::Center);
    }

    glEnable(GL_DEPTH_TEST);
}