#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

#include <string>
#include <memory>
#include <future>
#include <atomic>
#include <map>
#include <optional>

enum class EngineState;
class Scene;

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