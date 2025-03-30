#include "scene_manager/scene_manager.h"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"
#include "shader/shader.h"
#include "camera/camera.h"

std::shared_ptr<Scene> SceneManager::currentScene = nullptr;
std::future<std::shared_ptr<Scene>> SceneManager::pendingScene;
bool SceneManager::isLoading = false;

std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";
bool SceneManager::onTitleScreen = false;

void SceneManager::load(const std::string &sceneName)
{
    unload();

    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    currentScene = std::make_shared<Scene>(sceneMap[sceneName], sceneName);
    currentScene->uploadToGPU();

    Camera::reset();
    Physics::setup(*currentScene);
}

void SceneManager::loadAsync(const std::string &sceneName)
{
    unload();

    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    isLoading = true;

    std::future<std::shared_ptr<Scene>> futureScene = std::async(std::launch::async, [sceneName]()
                                                                 { return std::make_shared<Scene>(sceneMap[sceneName], sceneName); });

    pendingScene = std::move(futureScene);
}

void SceneManager::update()
{
    if (isLoading && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Now upload scene data to OpenGL (on main thread)
        currentScene->uploadToGPU();

        Camera::reset();
        Physics::setup(*currentScene);

        // Reset future
        pendingScene = std::future<std::shared_ptr<Scene>>();
        isLoading = false;
    }
    else if (!isLoading)
    {
        Physics::update(*currentScene);
        Animation::updateBones(*currentScene);
    }
}

void SceneManager::render()
{
    Camera::update();
    Render::render(*currentScene);
}

void SceneManager::unload()
{
    currentScene.reset();

    onTitleScreen = false;

    Shader::loadedShaders.clear();
    Shader::waterLoaded = false;
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

void SceneManager::renderLoading()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Render::renderText("Loading...", 0.1f, 0.85f, 1, glm::vec3(1.0f, 1.0f, 1.0f));
}