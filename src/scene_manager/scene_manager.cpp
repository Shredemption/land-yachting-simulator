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
#include "texture_manager/texture_manager.h"

// Global Scene variables
std::shared_ptr<Scene> SceneManager::currentScene = nullptr;
std::future<std::shared_ptr<Scene>> SceneManager::pendingScene;

// Scenemap and paths
std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";

// Global loading variables
std::atomic<EngineState> SceneManager::engineState = EngineState::Idle;
std::atomic<bool> SceneManager::updateCallbacks = false;
int SceneManager::loadingState = 0;
std::pair<std::atomic<int>, std::atomic<int>> SceneManager::loadingProgress = {0, 0};

bool SceneManager::enterPause = false;
bool SceneManager::exitPause = false;
float SceneManager::menuFade = 0.0f;

// Load scene on main, causes freezing
void SceneManager::load(const std::string &sceneName)
{
    engineState = EngineState::Loading;

    ThreadManager::stopRenderThread();

    // unload current scene
    unload();

    // Load scene from file
    currentScene = std::make_shared<Scene>(sceneMap[sceneName], sceneName);

    // Upload scene to GPU
    currentScene->uploadToGPU();

    // Setup cam and physics
    Camera::reset();
    Physics::setup(*currentScene);

    ThreadManager::startRenderThread();

    loadingState = 0;

    engineState = EngineState::Running;

    // Check if going to title
    if (sceneName == "title")
        engineState = EngineState::Title;

    updateCallbacks = true;
}

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    engineState = EngineState::Loading;

    loadingState++;

    ThreadManager::stopRenderThread();

    // Unload previous scene
    unload();

    loadingState++;

    // Future to store loaded scene in
    std::future<std::shared_ptr<Scene>> futureScene = std::async(std::launch::async, [sceneName]() -> std::shared_ptr<Scene>
                                                                 { 
                                                                    auto newScene = std::make_shared<Scene>(sceneMap[sceneName], sceneName); 
                                                                return newScene; });

    // Push future to global variable
    pendingScene = std::move(futureScene);
}

void SceneManager::checkLoading(GLFWwindow *window)
{
    // If background loading scene is complete
    if (loadingState > 0 && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Render final loading screen frame
        Render::renderLoading();

        glfwSwapBuffers(window);

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

        engineState = EngineState::Running;

        // Check if going to title
        if (currentScene.get()->name == "title")
            engineState = EngineState::Title;

        updateCallbacks = true;
    }
}

void SceneManager::unload()
{
    // Unload Texture array
    TextureManager::clearTextures();

    // Clear render buffers
    for (auto &buffer : Render::renderBuffers)
    {
        buffer.commandBuffer.clear();
        buffer.state.store(BufferState::Free);
    }

    // Reset scene variable. Calls destructors
    currentScene.reset();

    // Clear global data from loading before loading new scene
    Shader::unload();
    Shader::waterLoaded = false;

    SceneManager::menuFade = 0.0f;
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

void SceneManager::updateFade()
{
    float fadeTime = 0.2f; // seconds

    if (exitPause)
    {
        if (menuFade > 1.0f)
            menuFade = 1.0f;
            
        menuFade -= EventHandler::deltaTime / fadeTime;
        if (menuFade < 0.0f)
        {
            SceneManager::engineState = EngineState::Running;
            SceneManager::updateCallbacks = true;
            exitPause = false;
        }
    }
    else
    {
        menuFade += EventHandler::deltaTime / fadeTime;
        if (enterPause && menuFade >= 1.0f)
            enterPause = false;
    }
}