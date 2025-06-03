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
std::atomic<float> ThreadManager::animationAlpha(0.0f);
std::atomic<bool> ThreadManager::animationShouldExit(false);
std::condition_variable ThreadManager::animationCanWriteCV;
std::condition_variable ThreadManager::renderCanReadCV;
bool ThreadManager::animationDoneWriting = false;
bool ThreadManager::renderDoneReading = true;

std::mutex ThreadManager::renderBufferMutex;
std::condition_variable ThreadManager::renderBufferCV;
std::atomic<bool> ThreadManager::renderBufferShouldExit(false);
std::atomic<bool> ThreadManager::sceneReadyForRender(false);

void ThreadManager::startup()
{
    physicsThread = std::thread(physicsThreadFunction);
    animationThread = std::thread(animationThreadFunction);
    renderBufferThread = std::thread(renderBufferThreadFunction);

    {
        std::lock_guard<std::mutex> lock(animationMutex);
        animationCanWriteCV.notify_one();
        renderCanReadCV.notify_one();
    }
}

void ThreadManager::shutdown()
{
    // Tell threads to stop
    physicsShouldExit = true;
    animationShouldExit = true;
    renderBufferShouldExit = true;

    // Wake up threads so they can exit promptly
    physicsCV.notify_one();
    animationCanWriteCV.notify_all();
    renderCanReadCV.notify_all();
    renderBufferCV.notify_all();

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

        animationCanWriteCV.wait(lock, []
                                 { return renderDoneReading || animationShouldExit; });

        if (animationShouldExit)
            break;

        lock.unlock();

        float alpha = animationAlpha.load(std::memory_order_acquire);
        bool didAnimate = false;

        // Check if scene is valid before proceeding
        auto scenePtr = SceneManager::currentScene.get();
        if (scenePtr)
        {
            // Run animations sequentially for all models
            for (ModelData &model : scenePtr->structModels)
            {
                if (model.animated)
                {
                    auto &writeBones = model.model->getWriteBuffer();
                    Animation::updateYachtBones(model, alpha, writeBones);
                    didAnimate = true;
                }
            }
        }

        if (didAnimate)
            Model::swapBoneBuffers();

        lock.lock();
        animationDoneWriting = true;
        renderDoneReading = false;
        renderCanReadCV.notify_one();
        lock.unlock();
    }
}

void ThreadManager::renderBufferThreadFunction()
{
    while (!ThreadManager::renderBufferShouldExit.load())
    {
        std::unique_lock lock(renderBufferMutex);
        // Wait for scene to be loaded, and a buffer to be free
        renderBufferCV.wait(lock, []
                            { return renderBufferShouldExit ||
                                     (sceneReadyForRender.load(std::memory_order_acquire) &&
                                      SceneManager::currentScene &&
                                      std::any_of(std::begin(Render::renderBuffers), std::end(Render::renderBuffers),
                                                  [](const auto &b)
                                                  {
                                                      return b.state.load(std::memory_order_acquire) == BufferState::Free;
                                                  })); });

        if (renderBufferShouldExit || !sceneReadyForRender.load() || !SceneManager::currentScene)
            continue;

        // Find a free buffer to prepare
        int nextPrep = -1;
        for (int i = 0; i < 3; ++i)
        {
            if (Render::renderBuffers[i].state.load(std::memory_order_acquire) == BufferState::Free)
            {
                Render::renderBuffers[i].state.store(BufferState::Prepping, std::memory_order_release);
                nextPrep = i;
                break;
            }
        }

        Render::prepIndex.store(nextPrep, std::memory_order_release);
        lock.unlock();

        std::unique_lock<std::mutex> animationLock(animationMutex);
        renderCanReadCV.wait(animationLock, []
                             { return animationDoneWriting || renderBufferShouldExit; });

        if (renderBufferShouldExit)
            break;

        animationLock.unlock();

        // Fill command buffer for rendering
        Render::prepareRender(Render::renderBuffers[nextPrep]);

        // Mark as ready
        Render::renderBuffers[nextPrep].state.store(BufferState::Ready, std::memory_order_release);

        animationLock.lock();
        renderDoneReading = true;
        animationDoneWriting = false;
        animationCanWriteCV.notify_one();
        animationLock.unlock();

        // Notify that a buffer is ready
        renderBufferCV.notify_all();
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
    renderBufferThread = std::thread(renderBufferThreadFunction);
}