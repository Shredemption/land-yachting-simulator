#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

#include "scene/scene.h"

class Physics
{
public:
    Physics(ModelData &ModelData);
    static bool resetState;

    static glm::vec3 windDirection;
    static float windStrength;
    static float airDensity;
    static float g;

    static void setup(Scene &scene);
    static void update(Scene &scene);
    static void switchControlledYacht(Scene &scene);

    static bool keyInputs[5];

    glm::mat4 baseTransform;
    float steeringAngle = 0.0f;
    float steeringSmoothness;
    float maxSteeringAngle;
    float steeringAttenuation;
    float forwardVelocity = 0.0f;
    float wheelAngle = 0.0f;

    float maxMastAngle;
    float maxBoomAngle;
    float MastAngle = 0.0f;
    float BoomAngle = 0.0f;
    float sailControlFactor = 1.0f;
    float optimalAngle;
    float SailAngle;

    float maxLiftCoefficient;
    float minDragCoefficient;
    float rollCoefficient;
    float rollScaling;

    float sailArea;
    float mass;
    float bodyDragCoefficient;
    float bodyArea;

private:
    void move();
    void reset();
};

#endif