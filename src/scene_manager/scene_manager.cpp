#include "scene_manager/scene_manager.h"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>
#include <windows.h>

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"
#include "shader/shader.h"
#include "camera/camera.h"
#include "event_handler/event_handler.h"
#include "thread_manager/thread_manager.h"

// Global Scene variables
std::shared_ptr<Scene> SceneManager::currentScene = nullptr;
std::future<std::shared_ptr<Scene>> SceneManager::pendingScene;

// Scenemap and paths
std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";

// Global loading variables
std::atomic<bool> SceneManager::onTitleScreen(false);
int SceneManager::loadingState = 0;
std::pair<int, int> SceneManager::loadingProgress = {0, 0};

// Load scene on main, causes freezing
void SceneManager::load(const std::string &sceneName)
{
    ThreadManager::stopRenderThread();

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

    ThreadManager::startRenderThread();

    loadingState = 0;
}

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    loadingState++;

    ThreadManager::stopRenderThread();

    // Unload previous scene
    unload();

    // Check if going to title
    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    loadingState++;

    // Future to store loaded scene in
    std::future<std::shared_ptr<Scene>> futureScene = std::async(std::launch::async, [sceneName]() -> std::shared_ptr<Scene>
                                                                 { 
                                                                    auto newScene = std::make_shared<Scene>(sceneMap[sceneName], sceneName); 
                                                                return newScene; });

    // Push future to global variable
    pendingScene = std::move(futureScene);
}

void SceneManager::checkLoading()
{
    // If background loading scene is complete
    if (loadingState > 0 && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Render final loading screen frame
        SceneManager::renderLoading();

        // Now upload scene data to OpenGL
        currentScene->uploadToGPU();

        // Reset the camera and physics
        Camera::reset();
        Physics::setup(*currentScene);

        // Reset future
        pendingScene = std::future<std::shared_ptr<Scene>>();
        loadingState = 100;
    }
    // Briefly pause after loading to ensure completeness
    else if (loadingState == 100)
    {
        loadingState = 0;
        ThreadManager::startRenderThread();
    }
}

void SceneManager::unload()
{
    // Unload Texture array
    TextureManager::unloadTextures();

    // Reset scene variable. Calls destructors
    currentScene.reset();

    // Clear title sceen
    onTitleScreen = false;

    // Clear global data from loading before loading new scene
    Shader::unload();
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

    std::string progressString;
    std::string statusString = "Loading...";

    std::vector<LoadingStep> loadingSteps = {
        {"Unloaded Success", []
         { return "Clearing Previous"; }},
        {"Scene JSON Complete", []
         { return "Loading new Scene JSON"; }},
        {"Background Colors Complete", []
         { return "Loading Background Colors"; }},
        {"Texts Complete", [&]
         { return "Loading Texts [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Images Complete", [&]
         { return "Loading Images [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Models Complete", [&]
         { return "Loading Models [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Planes Complete", [&]
         { return "Loading Planes [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Terrain Grids Complete", [&]
         { return "Loading Terrain Grids [" + std::to_string(SceneManager::loadingProgress.first) + "/" + std::to_string(SceneManager::loadingProgress.second) + "]"; }},
        {"Skybox Complete", []
         { return "Loading Skybox"; }},
        {"OpenGL Upload Complete", []
         { return "Uploading to OpenGL"; }}};

    // Render completed steps
    for (int i = 0; i < loadingState - 1 && i < loadingSteps.size(); ++i)
    {
        progressString += loadingSteps[i].completedLabel + "\n";
    }

    // Render current step
    if (loadingState >= 1 && loadingState <= loadingSteps.size())
    {
        progressString += loadingSteps[loadingState - 1].activeMessage() + "\n";
    }

    // Loading complete
    if (loadingState == 100)
        statusString = "Finished Loading";

    Render::renderText(progressString, 0.05f, 0.05f, 0.85, glm::vec3(0.6f, 0.1f, 0.1f));
    Render::renderText(statusString, 0.05f, 0.9f, 1, glm::vec3(1.0f, 1.0f, 1.0f));
}