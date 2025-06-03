#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene/scene.h"

#include <future>
#include <atomic>

struct LoadingStep
{
    std::string completedLabel;
    std::function<std::string()> activeMessage;
};

enum class EngineState
{
    None,
    Loading,
    Title,
    Pause,
    Running
};

class SceneManager
{
public:
    // Global Scene variables
    static std::shared_ptr<Scene> currentScene;
    static std::future<std::shared_ptr<Scene>> pendingScene;

    // Scenemap and paths
    static std::map<std::string, std::string> sceneMap;
    static void loadSceneMap();

    // Global loading variables
    static std::atomic<bool> updateCallbacks;
    static int loadingState;
    static std::pair<std::atomic<int>, std::atomic<int>> loadingProgress;

    // Load scene
    static void load(const std::string &scenePath);
    static void loadAsync(const std::string &scenePath);
    static void unload();

    // Transition variables
    static EngineState engineState;
    static EngineState exitState;
    static float menuFade;
    static std::optional<std::string> upcomingSceneLoad;

    static void updateFade();
    static void switchEngineState(const EngineState &to);
    static void switchEngineStateScene(const std::string &sceneName);

    // Update and render functions
    static void checkLoading(GLFWwindow *window);
};

#endif