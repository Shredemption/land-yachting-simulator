#include "ui_manager/ui_manager.hpp"

#include "pch.h"

void UIManager::load(EngineState state)
{
    switch (state)
    {
    case EngineState::Title:
        UIManager::loadHTML("title.html");
        break;

    case EngineState::Settings:
    case EngineState::TitleSettings:
        UIManager::loadHTML("settings.html");
        break;

    case EngineState::Pause:
        UIManager::loadHTML("pause.html");
        break;
    }
}

void UIManager::updateHTML()
{
    Render::UL_view->FireMouseEvent({MouseEvent::kType_MouseMoved,
                                     static_cast<int>(InputManager::mousePosX),
                                     static_cast<int>(InputManager::mousePosY),
                                     MouseEvent::kButton_None});

    if (InputManager::leftMouseButton.pressed())
        Render::UL_view->FireMouseEvent({MouseEvent::kType_MouseDown,
                                         static_cast<int>(InputManager::mousePosX),
                                         static_cast<int>(InputManager::mousePosY),
                                         MouseEvent::kButton_Left});

    if (InputManager::leftMouseButton.released())
        Render::UL_view->FireMouseEvent({MouseEvent::kType_MouseUp,
                                         static_cast<int>(InputManager::mousePosX),
                                         static_cast<int>(InputManager::mousePosY),
                                         MouseEvent::kButton_Left});

    Render::UL_renderer->Update();
    Render::UL_renderer->Render();
    Render::UL_renderer->RefreshDisplay(0);
}

void UIManager::loadHTML(const std::string file)
{
    std::filesystem::path full_path = std::filesystem::absolute("resources/html/" + file);
    std::string url = "file:///" + full_path.string();
    std::replace(url.begin(), url.end(), '\\', '/');
    Render::UL_view->LoadURL(ultralight::String(url.c_str()));
}