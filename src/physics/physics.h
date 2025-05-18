#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

#include "scene/scene.h"

class Physics
{
public:
    // Constructor
    Physics(ModelData &ModelData);
    static bool resetState;

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

    // Boolmap for tracking inputs
    static bool keyInputs[5];

    // Velocity and steering variables
    glm::mat4 baseTransform;
    glm::mat4 transform;
    float steeringAngle = 0.0f;
    float steeringSmoothness;
    float maxSteeringAngle;
    float steeringAttenuation;
    float forwardVelocity = 0.0f;
    float wheelAngle = 0.0f;

    // Mast/sail/boom angles
    float maxMastAngle;
    float maxBoomAngle;
    float MastAngle = 0.0f;
    float BoomAngle = 0.0f;
    float sailControlFactor = 1.0f;
    float optimalAngle;
    float SailAngle = 0.0f;

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

#endif