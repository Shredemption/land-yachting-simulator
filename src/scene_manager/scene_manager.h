#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene/scene.h"

class SceneManager
{
public:
    static Scene *currentScene;

    static void load(const std::string &scenePath);
    static void update();
    static void render();
    static void unload();
};

#endif