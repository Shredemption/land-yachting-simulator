#include "scene_manager/scene_manager.h"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"
#include "shader/shader.h"
#include "camera/camera.h"

// Global Scene variables
std::shared_ptr<Scene> SceneManager::currentScene = nullptr;
std::future<std::shared_ptr<Scene>> SceneManager::pendingScene;
bool SceneManager::isLoading = false;

// Scenemap and paths
std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";

bool SceneManager::onTitleScreen = false;

// Load scene on main, causes freezing
void SceneManager::load(const std::string &sceneName)
{
    // unload current scene
    unload();

    // Check if going to title
    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    // Load scene from file
    currentScene = std::make_shared<Scene>(sceneMap[sceneName], sceneName);

    // Upload scene to GPU
    currentScene->uploadToGPU();

    // Setup cam and physics
    Camera::reset();
    Physics::setup(*currentScene);
}

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    // Unload previous scene
    unload();

    // Check if going to title
    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    isLoading = true;

    // Future to store loaded scene in
    std::future<std::shared_ptr<Scene>> futureScene = std::async(std::launch::async, [sceneName]()
                                                                 { return std::make_shared<Scene>(sceneMap[sceneName], sceneName); });

    // Push future to global variable
    pendingScene = std::move(futureScene);
}

void SceneManager::update()
{
    // If background loading scene is complete
    if (isLoading && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Now upload scene data to OpenGL
        currentScene->uploadToGPU();

        // Reset the camera and physics
        Camera::reset();
        Physics::setup(*currentScene);

        // Reset future
        pendingScene = std::future<std::shared_ptr<Scene>>();
        isLoading = false;
    }
    // If not loading, aka running normally
    else if (!isLoading)
    {
        Physics::update(*currentScene);
        Animation::updateBones(*currentScene);
    }
}

void SceneManager::render()
{
    // Update cam, render scene
    Camera::update();
    Render::render(*currentScene);
}

void SceneManager::unload()
{
    // Reset scene variable. Calls destructors
    currentScene.reset();

    // Clear title sceen 
    onTitleScreen = false;

    // Clear global data from loading before loading new scene
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
    // Render loading screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Render::renderText("Loading...", 0.1f, 0.85f, 1, glm::vec3(1.0f, 1.0f, 1.0f));
}