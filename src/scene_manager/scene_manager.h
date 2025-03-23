#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene/scene.h"

#include <thread>
#include <atomic>

class SceneManager
{
public:
    static Scene *currentScene;
    static std::atomic<bool> isLoading;
    static std::thread loadingThread;
    
    static std::map<std::string, std::string> sceneMap;
    static void loadSceneMap();

    static void load(const std::string &scenePath);
    static void update();
    static void render();
    static void unload();
    static void renderLoading();

    static bool onTitleScreen;
};

#endif