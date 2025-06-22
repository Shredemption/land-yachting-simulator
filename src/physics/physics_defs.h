#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <unordered_map>

struct YachtPhysicsPreset
{
    struct SailPreset
    {
        float maxMastAngle, maxBoomAngle, maxLiftCoefficient, minDragCoefficient, optimalAngle, sailArea;
    } sail;
    struct DrivingPreset
    {
        float rollCoefficient, rollScaling, steeringSmoothness, maxSteeringAngle, steeringAttenuation;

    } driving;
    struct BodyPreset
    {
        float mass, dragCoefficient, bodyArea;
    } body;
};

static const std::unordered_map<std::string, YachtPhysicsPreset> yachtPresets = {
    {"dn-duvel", {
                     {60, 90, 1.5, 0.1, 20, 6.0},
                     {0.01, 15, 3, 25, 0.5},
                     {250, 0.4, 1.2},
                 }},
    {"red-piper", {
                      {60, 90, 1.3, 0.1, 20, 6.8},
                      {0.01, 15, 3, 25, 0.45},
                      {180, 0.35, 1.0},
                  }},
    {"blue-piper", {
                       {60, 90, 1.4, 0.1, 20, 6.4},
                       {0.01, 15, 2, 25, 0.55},
                       {180, 0.35, 1.0},
                   }},
    {"sietske", {
                    {60, 90, 1.5, 0.1, 20, 5.7},
                    {0.008, 20, 2, 22.22, 1.0},
                    {200, 0.2, 1.0},
                }},
    {"bobbie", {
                   {40, 80, 1.0, 0.1, 20, 5.0},
                   {0.015, 20, 1, 30, 1.0},
                   {100, 0.1, 1.0},
               }},
    {"buizerd", {
                    {40, 80, 1.0, 0.1, 20, 5.0},
                    {0.015, 20, 1, 30, 1.0},
                    {100, 0.1, 1.0},
                }},
    {"beware", {
                   {40, 80, 1.0, 0.1, 20, 5.0},
                   {0.015, 20, 1, 30, 1.0},
                   {100, 0.1, 1.0},
               }},
    {"vampier", {
                    {40, 80, 1.0, 0.1, 20, 5.0},
                    {0.015, 20, 1, 30, 1.0},
                    {100, 0.1, 1.0},
                }},
};

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