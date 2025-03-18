#include "scene_manager/scene_manager.h"

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"

Scene *SceneManager::currentScene = nullptr;

void SceneManager::load(const std::string &scenePath)
{
    unload();

    currentScene = new Scene(scenePath);
    Physics::setup(*currentScene);
}

void SceneManager::update()
{
    Physics::update(*currentScene);
    Animation::updateBones(*currentScene);
}

void SceneManager::render()
{
    Render::render(*currentScene);
}

void SceneManager::unload()
{
    if (currentScene)
    {
        delete currentScene;
        currentScene = nullptr;
    }
}