#pragma once

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <memory>
#include <future>
#include <map>
#include <optional>

#include "scene_manager/scene_manager_defs.h"

class Scene;

namespace SceneManager
{
    // Global Scene variables
    inline std::shared_ptr<Scene> currentScene = nullptr;
    inline std::future<std::shared_ptr<Scene>> pendingScene;

    // Scenemap and paths
    inline std::map<std::string, std::string> sceneMap;
    inline std::string sceneMapPath = "resources/scenes.json";
    void loadSceneMap();

    // Global loading variables
    inline std::atomic<bool> updateCallbacks = true;
    inline int loadingState = 0;
    inline std::pair<std::atomic<int>, std::atomic<int>> loadingProgress = {0, 0};

    // Load scene
    void loadAsync(const std::string &scenePath);
    void unload();

    // Transition variables
    inline EngineState engineState = EngineState::None;

    void switchEngineState(const EngineState &to);
    void switchEngineStateScene(const std::string &sceneName);
    void switchSettingsPage(const SettingsPage &to);

    // Update and render functions
    void checkLoading();

    void runOneFrame();
};