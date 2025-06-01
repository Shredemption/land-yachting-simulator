#include "thread_manager/thread_manager.h"

#include "scene_manager/scene_manager.h"
#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"

// Define threads
std::thread ThreadManager::physicsThread;
std::thread ThreadManager::animationThread;
std::thread ThreadManager::renderBufferThread;

// Synchronisations
std::mutex ThreadManager::physicsMutex;
std::condition_variable ThreadManager::physicsCV;
std::atomic<bool> ThreadManager::physicsTrigger(false);
std::atomic<int> ThreadManager::physicsSteps(0);
std::atomic<bool> ThreadManager::physicsBusy(false);
std::atomic<bool> ThreadManager::physicsShouldExit(false);

std::mutex ThreadManager::animationMutex;
std::condition_variable ThreadManager::animationCV;
std::atomic<bool> ThreadManager::animationTrigger(false);
std::atomic<float> ThreadManager::animationAlpha(0.0f);
std::atomic<bool> ThreadManager::animationShouldExit(false);

std::mutex ThreadManager::renderBufferMutex;
std::condition_variable ThreadManager::renderBufferCV;
std::atomic<bool> ThreadManager::renderPrepReady(true);
std::atomic<bool> ThreadManager::renderExecuteReady(false);
std::atomic<bool> ThreadManager::renderBufferShouldExit(false);

void ThreadManager::startup()
{
    physicsThread = std::thread(physicsThreadFunction);
    animationThread = std::thread(animationThreadFunction);
    renderBufferThread = std::thread(renderBufferThreadFunction);
}

void ThreadManager::shutdown()
{
    // Tell threads to stop
    physicsShouldExit = true;
    animationShouldExit = true;
    renderBufferShouldExit = true;

    // Wake up threads so they can exit promptly
    physicsCV.notify_one();
    animationCV.notify_one();
    renderBufferCV.notify_one();

    // Join threads back to main
    if (physicsThread.joinable())
        physicsThread.join();
    if (animationThread.joinable())
        animationThread.join();
    if (renderBufferThread.joinable())
        renderBufferThread.join();
}

void ThreadManager::physicsThreadFunction()
{
    while (!physicsShouldExit)
    {
        std::unique_lock<std::mutex> lock(physicsMutex);
        physicsCV.wait(lock, []
                       { return physicsTrigger.load() || physicsShouldExit.load(); });

        if (physicsShouldExit)
            break;

        physicsTrigger = false;

        for (ModelData &model : SceneManager::currentScene.get()->structModels)
        {
            if (model.physics.has_value())
            {
                model.physics->getWriteBuffer()->copyFrom(*model.physics->getReadBuffer());
                model.physics->getWriteBuffer()->savePrevState();
            }
        }

        while (true)
        {
            int oldSteps = physicsSteps.load();
            if (oldSteps <= 0)
                break;

            if (physicsSteps.compare_exchange_weak(oldSteps, oldSteps - 1))
            {
                for (ModelData &model : SceneManager::currentScene.get()->structModels)
                {
                    if (model.physics.has_value())
                    {
                        model.physics->getWriteBuffer()->move(model.controlled);
                    }
                }

                Physics::accumulator.store(Physics::accumulator.load(std::memory_order_acquire) - Physics::tickRate);
            }
        }

        for (auto &model : SceneManager::currentScene->structModels)
        {
            Physics::isSwapping.store(true, std::memory_order_release);

            if (model.physics)
                model.physics->swapBuffers();

            Physics::isSwapping.store(false, std::memory_order_release);
        }

        ThreadManager::physicsBusy.store(false, std::memory_order_release);
    }
}

void ThreadManager::animationThreadFunction()
{
    while (!animationShouldExit)
    {
        std::unique_lock<std::mutex> lock(animationMutex);
        animationCV.wait(lock, []
                         { return animationTrigger.load() || animationShouldExit.load(); });

        if (animationShouldExit)
            break;

        animationTrigger = false;

        float alpha = animationAlpha.load(std::memory_order_acquire);

        // Run animations sequentially for all models
        for (ModelData &model : SceneManager::currentScene.get()->structModels)
        {
            if (model.animated)
            {
                Animation::updateYachtBones(model, alpha);
            }
        }
    }
}

void ThreadManager::renderBufferThreadFunction()
{
    while (!renderBufferShouldExit)
    {
        {
            std::unique_lock<std::mutex> lock(renderBufferMutex);
            renderBufferCV.wait(lock, []
                                { return renderPrepReady || renderBufferShouldExit; });

            if (renderBufferShouldExit)
                break;

            renderPrepReady = false;
        }

        Render::prepareRender();

        {
            std::lock_guard<std::mutex> lock(renderBufferMutex);
            renderExecuteReady = true;
        }
        renderBufferCV.notify_one();
    }
}

void ThreadManager::stopRenderThread()
{
    {
        std::lock_guard<std::mutex> lock(renderBufferMutex);
        renderBufferShouldExit = true;
        renderBufferCV.notify_all();
    }
    if (renderBufferThread.joinable())
        renderBufferThread.join();
}

void ThreadManager::startRenderThread()
{
    renderBufferShouldExit = false;
    renderPrepReady = true;
    renderExecuteReady = false;
    renderBufferThread = std::thread(renderBufferThreadFunction);
}