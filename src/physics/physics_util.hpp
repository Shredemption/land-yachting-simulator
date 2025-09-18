#pragma once

#include <glm/glm.hpp>

#include <atomic>

struct Scene;
struct ModelData;

inline void atomicAdd(std::atomic<double> &atomicVal, double value)
{
    double current = atomicVal.load(std::memory_order_relaxed);
    double desired;
    do
    {
        desired = current + value;
    } while (!atomicVal.compare_exchange_weak(current, desired, std::memory_order_release, std::memory_order_relaxed));
}

namespace PhysicsUtil
{
    void update();

    inline std::atomic<double> accumulator = 0.0;

    inline std::atomic<bool> isSwapping(false);

    // World variables
    inline glm::vec3 windDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    inline float windStrength = 10.0f;
    inline float airDensity = 1.225f;
    inline float g = 9.80665f;

    // Functions
    void setup();
    void stepPhysics(ModelData &model);
    void switchControlledYacht();

    // Boolmap for tracking inputs
    inline bool keyInputs[5];

};