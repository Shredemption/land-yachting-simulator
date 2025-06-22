#include "physics/physics_util.hpp"

#include "pch.h"

void PhysicsUtil::update()
{
    atomicAdd(accumulator, TimeManager::deltaTime);

    int steps = 0;
    double acc = accumulator.load(std::memory_order_acquire);
    double tickTime = 1 / SettingsManager::settings.physics.tickRate;

    while (acc >= tickTime)
    {
        acc -= tickTime;
        steps++;
    }

    if (steps > 0 && !ThreadManager::physicsBusy.load(std::memory_order_acquire))
    {
        ThreadManager::physicsBusy.store(true, std::memory_order_release);
        // Update physics
        {
            std::lock_guard lock(ThreadManager::physicsMutex);
            ThreadManager::physicsTrigger = true;
            ThreadManager::physicsSteps += steps;
        }
        ThreadManager::physicsCV.notify_one();
    }

    float alpha = static_cast<float>(acc / tickTime);
    ThreadManager::animationAlpha.store(alpha, std::memory_order_release);
}

void PhysicsUtil::stepPhysics(ModelData &model)
{
    auto *read = model.physics->getReadBuffer();
    auto *write = model.physics->getWriteBuffer();

    write->update(model);

    if (write->onGround)
    {
        read->base.pos = write->base.pos;
        read->base.vel = write->base.vel;
    }
}

void PhysicsUtil::setup()
{
    // Setup all animated models
    for (ModelData &model : SceneManager::currentScene.get()->structModels)
    {
        if (model.physicsTypes.size() > 0)
        {
            model.physics.emplace();

            model.physics->buffers[0] = std::make_unique<Physics>(model);
            model.physics->buffers[1] = std::make_unique<Physics>(model);

            model.physics->buffers[0]->reset(model.u_model);
            model.physics->buffers[1]->reset(model.u_model);
        }
    }
}

void PhysicsUtil::switchControlledYacht()
{
    std::string current;
    Scene *scene = SceneManager::currentScene.get();

    // Find current controlled yacht, and stop controlling it
    for (auto &model : scene->structModels)
    {
        if (model.controlled)
        {
            current = model.model->name;
            model.controlled = false;
        }
    }

    // Find id of current yacht in loaded yachts, increment by 1, overflow
    int currentId = find(scene->loadedYachts.begin(), scene->loadedYachts.end(), current) - scene->loadedYachts.begin();
    int newId = (currentId + 1) % scene->loadedYachts.size();

    // Control new yacht
    for (auto &model : scene->structModels)
    {
        if (model.model->name == scene->loadedYachts[newId])
        {
            model.controlled = true;
        }
    }
}