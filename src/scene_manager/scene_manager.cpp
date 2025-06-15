#include "scene_manager/scene_manager.hpp"

#include "pch.h"

// Load scene in background, show loading screen
void SceneManager::loadAsync(const std::string &sceneName)
{
    switchEngineState(EngineState::Loading);

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

void SceneManager::checkLoading()
{
    // If background loading scene is complete
    if (loadingState > 0 && pendingScene.valid() && pendingScene.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
    {
        // Retrieve the loaded scene
        currentScene = pendingScene.get();

        // Render final loading screen frame
        Render::renderLoadingScreen();

        glfwSwapBuffers(WindowManager::window);

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

        switchEngineState(EngineState::Running);
    }
}

void SceneManager::runOneFrame()
{
    if (!currentScene)
        return;

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
    const std::string path = sceneMapPath;
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
    if (engineState == EngineState::Settings || engineState == EngineState::TitleSettings)
    {
        SettingsManager::save();
    }

    switch (to)
    {
    case EngineState::Settings:
        UIManager::loadHTML("settings.html");
        break;
    case EngineState::TitleSettings:
        UIManager::loadHTML("titlesettings.html");
    }

    engineState = to;
}

void SceneManager::switchEngineStateScene(const std::string &sceneName)
{
    switchEngineState(EngineState::Loading);
    loadAsync(sceneName);
}