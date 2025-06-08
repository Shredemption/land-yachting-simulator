#pragma once

#include <chrono>
#include <optional>

enum class EngineState;

namespace TimeManager
{
    inline double time = 0.0;
    inline double deltaTime = 0.0;
    inline std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    inline std::chrono::steady_clock::time_point lastTime = startTime;
    inline std::chrono::steady_clock::time_point now = startTime;
    inline unsigned int frame = 0;

    inline std::optional<std::chrono::steady_clock::time_point> pauseStart{};
    inline std::chrono::duration<double> pausedDuration{0};

    void timing(EngineState &state);
};