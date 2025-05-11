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

// Global Scene variables
std::shared_ptr<Scene> SceneManager::currentScene = nullptr;
std::future<std::shared_ptr<Scene>> SceneManager::pendingScene;

// Scenemap and paths
std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";

// Global loading variables
bool SceneManager::onTitleScreen = false;
int SceneManager::loadingState = 0;
std::pair<int, int> SceneManager::loadingProgress = {0, 0};

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

    loadingState = 0;
}

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    loadingState++;

    // Unload previous scene
    unload();

    // Check if going to title
    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    loadingState++;

    // Future to store loaded scene in
    std::future<std::shared_ptr<Scene>> futureScene = std::async(std::launch::async, [sceneName]()
                                                                 { return std::make_shared<Scene>(sceneMap[sceneName], sceneName); });

    // Push future to global variable
    pendingScene = std::move(futureScene);
}

void SceneManager::update()
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
        Sleep(500);
    }
    // If not loading, aka running normally
    else if (loadingState == 0)
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
    // Unload Texture array
    Model::unloadTextures();

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
    std::string progressString;
    std::string statusString = "Loading...";
    // Render loading screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Completed loading steps
    if (loadingState > 1)
        progressString += "Unloaded Success\n";
    if (loadingState > 2)
        progressString += "Scene JSON Complete\n";
    if (loadingState > 3)
        progressString += "Background Colors Complete\n";
    if (loadingState > 4)
        progressString += "Texts Complete\n";
    if (loadingState > 5)
        progressString += "Models Complete\n";
    if (loadingState > 6)
        progressString += "Planes Complete\n";
    if (loadingState > 7)
        progressString += "Terrain Grids Complete\n";
    if (loadingState > 8)
        progressString += "Skybox Complete\n";
    if (loadingState > 9)
        progressString += "OpenGL Upload Complete\n";

    // Current loading step
    if (loadingState == 1)
        progressString += "Clearing Previous\n";
    else if (loadingState == 2)
        progressString += "Loading new Scene JSON\n";
    else if (loadingState == 3)
        progressString += "Loading Background Colors\n";
    else if (loadingState == 4)
        progressString += "Loading Texts [" + std::to_string(loadingProgress.first) + "/" + std::to_string(loadingProgress.second) + "]\n";
    else if (loadingState == 5)
        progressString += "Loading Models [" + std::to_string(loadingProgress.first) + "/" + std::to_string(loadingProgress.second) + "]\n";
    else if (loadingState == 6)
        progressString += "Loading Planes [" + std::to_string(loadingProgress.first) + "/" + std::to_string(loadingProgress.second) + "]\n";
    else if (loadingState == 7)
        progressString += "Loading Terrain Grids [" + std::to_string(loadingProgress.first) + "/" + std::to_string(loadingProgress.second) + "]\n";
    else if (loadingState == 8)
        progressString += "Loading Skybox\n";
    else if (loadingState == 9)
        progressString += "Uploading to OpenGL\n";

    // If loading complete
    if (loadingState == 100)
        statusString = "Finished Loading";

    Render::renderText(progressString, 0.05f, 0.05f, 0.85, glm::vec3(0.6f, 0.1f, 0.1f));

    Render::renderText(statusString, 0.05f, 0.9f, 1, glm::vec3(1.0f, 1.0f, 1.0f));
}