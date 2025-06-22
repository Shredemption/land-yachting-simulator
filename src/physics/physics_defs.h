#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct BaseVariables
{
    glm::vec3 pos;
    glm::vec3 prevPos;

    glm::vec3 vel = glm::vec3(0.0f);
    glm::vec3 acc = glm::vec3(0.0f);

    float mass = 1.0f;

    glm::vec3 netForce;

    glm::quat rot;
    glm::quat prevRot;
};

struct BodyVariables
{
    float dragCoefficient;
    float area;
};

struct SailVariables
{
    float area;
    float maxLiftCoefficient;
    float minDragCoefficient;

    float maxMastAngle;
    float maxBoomAngle;
    float optimalAngle;

    float controlFactor = 1.0f;

    float MastAngle = 0.0f, prevMastAngle;
    float BoomAngle = 0.0f, prevBoomAngle;
    float SailAngle = 0.0f, prevSailAngle;
};

struct DrivingVariables
{
    float steeringSmoothness;
    float maxSteeringAngle;
    float steeringAttenuation;
    float steeringChange;

    float steeringAngle = 0.0f, prevSteeringAngle;
    float wheelAngle = 0.0f, prevWheelAngle;

    float rollCoefficient;
    float rollScaling;
};

struct CollisionVariables
{
    bool ground = false;
    int collisions;
};

struct DebugForce
{
    glm::vec3 position;
    glm::vec3 force;
    std::string label;
};

enum class PhysicsType
{
    Body,
    Driving,
    Sail,
    Gravity,
    Collision
};