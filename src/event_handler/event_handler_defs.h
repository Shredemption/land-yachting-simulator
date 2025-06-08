#pragma once

struct MouseButtonState
{
    bool isDown;
    bool wasDown;

    bool pressed() const { return isDown && !wasDown; }
    bool released() const { return !isDown && wasDown; }
    bool held() const { return isDown; }
};

enum class InputType
{
    itKeyboard,
    itMouse,
    itController
};