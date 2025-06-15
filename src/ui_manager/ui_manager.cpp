#include "ui_manager/ui_manager.hpp"

#include "pch.h"

#include <Ultralight/JavaScript.h>
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSObjectRef.h>

JSValueRef SwitchEngineStateCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
    if (argumentCount < 1 || !JSValueIsString(ctx, arguments[0]))
    {
        std::cerr << "Expected one string argument." << std::endl;
        return JSValueMakeUndefined(ctx);
    }

    JSStringRef jsStr = JSValueToStringCopy(ctx, arguments[0], exception);
    size_t size = JSStringGetMaximumUTF8CStringSize(jsStr);
    std::vector<char> buffer(size);
    JSStringGetUTF8CString(jsStr, buffer.data(), size);
    std::string arg(buffer.data());
    JSStringRelease(jsStr);

    SceneManager::switchEngineState(parseEngineState(arg));

    return JSValueMakeUndefined(ctx);
};

void BindJSFunctions(RefPtr<View> view)
{
    JSContextRef ctx = view->LockJSContext()->ctx();
    JSObjectRef global = JSContextGetGlobalObject(ctx);

    JSStringRef name = JSStringCreateWithUTF8CString("switchEngineState");
    JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, name, SwitchEngineStateCallback);

    JSObjectSetProperty(ctx, global, name, func, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(name);
}

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

    while (Render::UL_view->is_loading())
        Render::UL_renderer->Update();

    BindJSFunctions(Render::UL_view);
}