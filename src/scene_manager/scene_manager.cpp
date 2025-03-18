#include "scene_manager/scene_manager.h"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"

Scene *SceneManager::currentScene = nullptr;
std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";
bool SceneManager::onTitleScreen = false;

void SceneManager::load(const std::string &sceneName)
{
    unload();

    currentScene = new Scene(sceneMap[sceneName], sceneName);
    Physics::setup(*currentScene);

    if (sceneName == "title")
    {
        onTitleScreen = true;
    }
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

    onTitleScreen = false;
}

void SceneManager::loadSceneMap()
{
    const std::string path = "../" + sceneMapPath;
    // Check if the file exists
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File not found: " + path);
    }

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path);
    }

    // Parse the JSON
    jsoncons::json j;
    try
    {
        j = jsoncons::json::parse(file);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    // Create a map from the parsed JSON
    for (const auto &kv : j["scenes"].object_range())
    {
        sceneMap[kv.key()] = kv.value().as<std::string>();
    }
}