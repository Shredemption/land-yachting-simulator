#include "time_manager/time_manager.hpp"

#include "pch.h"

void TimeManager::timing(EngineState &state)
{
    now = std::chrono::steady_clock::now();

    std::chrono::duration<double> delta = now - lastTime;
    deltaTime = delta.count();

    lastTime = now;
    frame++;

    bool timeShouldPause = (state == EngineState::esPause || state == EngineState::esLoading || state == EngineState::esSettings || state == EngineState::esTitleSettings);

    if (timeShouldPause)
    {
        if (!pauseStart.has_value())
            pauseStart = now;
    }
    else
    {
        if (pauseStart.has_value())
        {
            pausedDuration += now - *pauseStart;
            pauseStart.reset();
        }

        std::chrono::duration<double> total = now - startTime - pausedDuration;
        time = total.count();
    }
}