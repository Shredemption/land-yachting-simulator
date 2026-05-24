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
        g_renderer->renderLoadingScreen();

        WindowManager::swapBuffers();

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
            model.physics->getWriteBuffer()->update(model);
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
    g_renderer->prepareRender(g_renderer->renderBuffers[0]);
    g_renderer->executeRender(g_renderer->renderBuffers[0], false);
}

void SceneManager::unload()
{
    // Unload Texture array
    TextureAssetManager::clear();

    ThreadManager::sceneReadyForRender.store(false, std::memory_order_release);

    // Clear render buffers
    for (auto &buffer : g_renderer->renderBuffers)
    {
        buffer.commandBuffer.clear();
        buffer.state.store(BufferState::Free);
    }

    // Reset scene variable. Calls destructors
    currentScene.reset();

    // Clear global data from loading before loading new scene
    ShaderUtil::unload();
}

void SceneManager::initSceneMap()
{
    std::cout << "[SceneManager] Initializing scene registry" << std::endl;

    sceneMap = {
        {"cartoon", "resources/scenes/cartoon.json"},
        {"realistic", "resources/scenes/realistic.json"},
        {"test-yacht", "resources/scenes/test/yacht.json"},
        {"test-rigid", "resources/scenes/test/rigid-body.json"}};

    std::cout << "[SceneManager] Scene registry initialized (" << sceneMap.size() << " scenes)" << std::endl;
}

void SceneManager::switchEngineState(const EngineState &to)
{
    if (engineState == EngineState::Settings || engineState == EngineState::TitleSettings)
    {
        SettingsManager::save();
    }

    engineState = to;

    updateCallbacks = true;

    switch (to)
    {
    case EngineState::Title:
    {
        unload();
        TextureAssetManager::queueStandaloneImage("title-figure.png");
        TextureAssetManager::queueStandaloneImage("title-figure-black.png");
        TextureAssetManager::loadQueuedPixelData();
        g_renderer->getTextureManager()->uploadPending();
        break;
    }
    case EngineState::Pause:
    {
        g_renderer->savePauseBackground();
        break;
    }
    case EngineState::Settings:
    case EngineState::TitleSettings:
    {
        settingsPage = SettingsPage::Start;
        break;
    }
    }

    UIManager::load(to);
}

void SceneManager::switchEngineStateScene(const std::string &sceneName)
{
    switchEngineState(EngineState::Loading);
    loadAsync(sceneName);
}