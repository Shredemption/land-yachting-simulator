#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>
#include <atomic>
#include <memory>

struct Scene;
struct ModelData;

class Physics
{
public:
    // Constructor
    Physics(ModelData &ModelData);
    static bool resetState;

    static const double tickRate;
    static std::atomic<double> accumulator;

    // World variables
    static glm::vec3 windSourceDirection;
    static float windStrength;
    static float airDensity;
    static float g;

    // Functions
    static void setup(Scene &scene);
    void move(bool &controlled);
    void reset(ModelData &modelData);
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

    Physics *getReadBuffer() { return buffers[readIndex % 2].get(); }
    Physics *getWriteBuffer() { return buffers[(readIndex + 1) % 2].get(); }

    void swapBuffers() { readIndex = (readIndex + 1) % 2; }
};

#endif