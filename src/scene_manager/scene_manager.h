#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene/scene.h"

#include <future>

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
    static bool onTitleScreen;
    static int loadingState;
    static std::pair<int, int> loadingProgress;

    // Load scene
    static void load(const std::string &scenePath);
    static void loadAsync(const std::string &scenePath);
    static void unload();

    // Update and render functions
    static void checkLoading();
    static void render();
    static void renderLoading();
};

#endif