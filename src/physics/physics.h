#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>
#include <atomic>
#include <memory>
#include <thread>

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

class Physics
{
public:
    // Constructor
    Physics(const std::string &name);
    static void update();

    static bool resetState;

    static const double tickRate;
    static std::atomic<double> accumulator;

    static std::atomic<bool> isSwapping;

    // World variables
    static glm::vec3 windSourceDirection;
    static float windStrength;
    static float airDensity;
    static float g;

    // Functions
    static void setup(Scene &scene);
    void move(bool &controlled);
    void reset(const glm::mat4 &u_model);
    static void switchControlledYacht(Scene &scene);
    void savePrevState();
    void copyFrom(const Physics &other);

    // Boolmap for tracking inputs
    static bool keyInputs[5];

    // Velocity and steering variables
    glm::mat4 baseTransform, prevBaseTransform;
    float steeringAngle = 0.0f, prevSteeringAngle;
    float steeringSmoothness;
    float maxSteeringAngle;
    float steeringAttenuation;
    float forwardVelocity = 0.0f;
    float wheelAngle = 0.0f, prevWheelAngle;

    // Mast/sail/boom angles
    float maxMastAngle;
    float maxBoomAngle;
    float MastAngle = 0.0f, prevMastAngle;
    float BoomAngle = 0.0f, prevBoomAngle;
    float sailControlFactor = 1.0f;
    float optimalAngle;
    float SailAngle = 0.0f, prevSailAngle;

    // Friction/drag coefficients
    float maxLiftCoefficient;
    float minDragCoefficient;
    float rollCoefficient;
    float rollScaling;

    // Body size properties
    float sailArea;
    float mass;
    float bodyDragCoefficient;
    float bodyArea;
};

struct PhysicsBuffer
{
    std::unique_ptr<Physics> buffers[2];
    int readIndex = 0;

    Physics *getReadBuffer()
    {
        while (Physics::isSwapping.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }
        return buffers[readIndex % 2].get();
    }

    Physics *getWriteBuffer() { return buffers[(readIndex + 1) % 2].get(); }

    void swapBuffers() { readIndex = (readIndex + 1) % 2; }
};

#endif