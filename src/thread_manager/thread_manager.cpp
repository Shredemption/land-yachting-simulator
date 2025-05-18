#include "thread_manager/thread_manager.h"

#include "scene_manager/scene_manager.h"
#include "physics/physics.h"
#include "animation/animation.h"

// Define threads
std::thread ThreadManager::physicsThread;
std::thread ThreadManager::animationThread;

// Synchronisations
std::mutex ThreadManager::physicsMutex;
std::condition_variable ThreadManager::physicsCV;
std::atomic<bool> ThreadManager::physicsTrigger(false);
std::atomic<bool> ThreadManager::physicsShouldExit(false);

std::mutex ThreadManager::animationMutex;
std::condition_variable ThreadManager::animationCV;
std::atomic<bool> ThreadManager::animationTrigger(false);
std::atomic<bool> ThreadManager::animationShouldExit(false);

void ThreadManager::startup()
{
    physicsThread = std::thread(physicsThreadFunction);
    animationThread = std::thread(animationThreadFunction);
}

void ThreadManager::shutdown()
{
    // Tell threads to stop
    physicsShouldExit = true;
    animationShouldExit = true;

    // Wake up threads so they can exit promptly
    physicsCV.notify_one();
    animationCV.notify_one();

    // Join threads back to main
    if (physicsThread.joinable())
        physicsThread.join();
    if (animationThread.joinable())
        animationThread.join();
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
            if (!model.physics.empty())
            {
                model.physics[0]->move(model.controlled);
            }
        }
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

        animationTrigger = false; // reset trigger

        float alpha = Physics::accumulator / Physics::tickRate;

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