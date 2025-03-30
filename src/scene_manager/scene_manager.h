#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene/scene.h"

#include <future>

class SceneManager
{
public:
    static std::shared_ptr<Scene> currentScene;
    static std::future<std::shared_ptr<Scene>> pendingScene;
    static bool isLoading;

    static std::map<std::string, std::string> sceneMap;
    static void loadSceneMap();

    static void load(const std::string &scenePath);
    static void loadAsync(const std::string &scenePath);
    static void update();
    static void render();
    static void unload();
    static void renderLoading();

    static bool onTitleScreen;
};

#endif