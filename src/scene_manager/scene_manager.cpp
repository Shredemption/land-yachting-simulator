#include "scene_manager/scene_manager.hpp"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "animation/animation.hpp"
#include "camera/camera.hpp"
#include "event_handler/event_handler.hpp"
#include "model/model.hpp"
#include "model/model_util.hpp"
#include "physics/physics_util.hpp"
#include "render/render.hpp"
#include "scene/scene.hpp"
#include "scene_manager/scene_manager_defs.h"
#include "shader/shader_util.hpp"
#include "texture_manager/texture_manager.hpp"
#include "thread_manager/thread_manager.hpp"
#include "ui_manager/ui_manager.hpp"

std::string sceneMapPath = "resources/scenes.json";

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    switchEngineState(EngineState::esLoading);

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
    if (upcomingSceneLoad.has_value())
    {
        loadAsync(upcomingSceneLoad.value());
        upcomingSceneLoad.reset();
    }

    // If background loading scene is complete
    if (loadingState > 0 && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Render final loading screen frame
        Render::renderLoadingScreen();

        glfwSwapBuffers(window);

        // Now upload scene data to OpenGL
        currentScene->uploadToGPU();

        // Reset the camera and physics
        Camera::reset();
        PhysicsUtil::setup();

        runOneFrame();

        // Reset future
        pendingScene = std::future<std::shared_ptr<Scene>>();
        loadingState = 100;
    }

    else if (loadingState == 100)
    {
        loadingState = 0;
        ThreadManager::sceneReadyForRender.store(true, std::memory_order_release);
        ThreadManager::startRenderThread();

        switchEngineState(EngineState::esRunning);
    }
}

void SceneManager::runOneFrame()
{
    // Run one physics tick
    for (ModelData &model : currentScene.get()->structModels)
    {
        if (model.physics.has_value())
        {
            model.physics->getWriteBuffer()->copyFrom(*model.physics->getReadBuffer());
            model.physics->getWriteBuffer()->savePrevState();
            model.physics->getWriteBuffer()->move(model.controlled);
            model.physics->swapBuffers();
        }
    }

    // Update all bones once
    for (ModelData &model : currentScene.get()->structModels)
    {
        if (model.animated)
        {
            auto &writeBones = model.model->getWriteBuffer();
            Animation::updateYachtBones(model, 1.0f, writeBones);
        }
    }

    ModelUtil::swapBoneBuffers();

    // Render one frame to buffer
    Render::prepareRender(Render::renderBuffers[0]);
    Render::executeRender(Render::renderBuffers[0], false);
}

void SceneManager::unload()
{
    // Unload Texture array
    TextureManager::clearTextures();

    ThreadManager::sceneReadyForRender.store(false, std::memory_order_release);

    // Clear render buffers
    for (auto &buffer : Render::renderBuffers)
    {
        buffer.commandBuffer.clear();
        buffer.state.store(BufferState::Free);
    }

    // Reset scene variable. Calls destructors
    currentScene.reset();

    // Clear global data from loading before loading new scene
    ShaderUtil::unload();
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

void SceneManager::switchEngineState(const EngineState &to)
{
    exitState = engineState;
    engineState = to;

    if (to == EngineState::esTitle)
    {
        TextureManager::queueStandaloneImage("title-figure.png");
        TextureManager::queueStandaloneImage("title-figure-black.png");
        TextureManager::loadQueuedPixelData();
        TextureManager::uploadToGPU();
    }
}

void SceneManager::switchSettingsPage(const SettingsPage &to)
{
    exitPage = settingsPage;
    settingsPage = to;
}

void SceneManager::switchEngineStateScene(const std::string &sceneName)
{
    switchEngineState(EngineState::esLoading);

    upcomingSceneLoad.emplace(sceneName);
}

void SceneManager::updateFade()
{
    const float fadeTime = 0.2f; // seconds
    float fadeDelta = EventHandler::deltaTime / fadeTime;

    switch (exitState)
    {
    // Fade up if not exiting
    case EngineState::esNone:
        if (engineState == EngineState::esSettings || engineState == EngineState::esTitleSettings)
            switch (exitPage)
            {
            case SettingsPage::spNone:
                menuFade += fadeDelta;
                break;

            default:
                menuFade = std::clamp(menuFade - fadeDelta, 0.0f, 1.0f);

                if (menuFade <= 0.0f)
                {
                    exitPage = SettingsPage::spNone;
                    UIManager::load(settingsPage);
                }
                break;
            }
        else
            menuFade += fadeDelta;
        break;

    // Instant fade on exit Running
    case EngineState::esRunning:
        menuFade = 0.0f;
        exitState = EngineState::esNone;
        UIManager::load(engineState);
        updateCallbacks = true;
        break;

    // Else, fade out
    default:
        menuFade = std::clamp(menuFade - fadeDelta, 0.0f, 1.0f);

        if (menuFade <= 0.0f)
        {
            exitState = EngineState::esNone;
            UIManager::load(engineState);
            updateCallbacks = true;
        }
        break;
    }
}