#include <physics/physics.h>

#include "event_handler/event_handler.h"
#include "render/render.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

bool Physics::keyInputs[5];
glm::vec3 Physics::windDirection = glm::vec3(0.0f, 1.0f, 0.0f);
float Physics::windStrength = 15;
float Physics::airDensity = 1.225;
float Physics::g = 9.81;

bool Physics::resetState = false;

Physics::Physics(ModelData &ModelData)
{
    baseTransform = glm::mat4(1.0f);

    if (ModelData.model->path.find("duvel/duvel") != std::string::npos)
    {
        maxMastAngle = glm::radians(30.0f);
        maxBoomAngle = glm::radians(90.0f);
        sailControlFactor = 1.0f;
        maxLiftCoefficient = 1.5;
        minDragCoefficient = 0.1;
        sailArea = 8;
        rollCoefficient = 0.01;
        mass = 300;

        steeringSmoothness = 3.0;
        maxSteeringAngle = 10;
    }
}

void Physics::reset()
{
    baseTransform = glm::mat4(1.0f);
    sailControlFactor = 1.0f;
    MastAngle = 0.0f;
    targetMastAngle = 0.0f;
    BoomAngle = 0.0f;
    targetBoomAngle = 0.0f;
    steeringAngle = 0.0f;
    steeringChange = 0.0f;
    forwardVelocity = 0.0f;
    forwardAcceleration = 0.0f;
}

void Physics::setup(Scene &scene)
{
    for (int i = 0; i < scene.structModels.size(); i++)
    {
        if (scene.structModels[i].type == "yacht_controlled")
        {
            scene.structModels[i].physics.push_back(new Physics(scene.structModels[i]));
        }
    }
}

void Physics::update(Scene &scene)
{
    for (int i = 0; i < scene.structModels.size(); i++)
    {
        if (scene.structModels[i].type == "yacht_controlled")
        {
            scene.structModels[i].physics[0]->move();
            scene.structModels[i].physics[0]->debug();
        }
    }
}

void Physics::move()
{
    if (resetState)
    {
        resetState = false;
        this->reset();
    }
    // Acceleration from keys
    forwardAcceleration = 0.0f;
    steeringChange = 0.0f;

    if (keyInputs[0])
    {
        sailControlFactor += 1.f * EventHandler::deltaTime;
    }
    if (keyInputs[1])
    {
        sailControlFactor -= 0.4f * EventHandler::deltaTime;
    }
    if (keyInputs[2])
    {
        steeringChange += steeringSmoothness * maxSteeringAngle;
    }
    if (keyInputs[3])
    {
        steeringChange -= steeringSmoothness * maxSteeringAngle;
    }
    if (keyInputs[4])
    {
        forwardAcceleration += 2.f;
    }

    sailControlFactor = std::clamp(sailControlFactor, 0.0f, 1.0f);

    // Find new angles for sail
    glm::vec3 direction = glm::normalize(glm::vec3(baseTransform[1]));
    angleToWind = glm::orientedAngle(direction, windDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    targetMastAngle = sailControlFactor * std::clamp(angleToWind, -maxMastAngle, maxMastAngle);
    targetBoomAngle = sailControlFactor * std::clamp(angleToWind, -maxBoomAngle + maxMastAngle, maxBoomAngle - maxMastAngle);

    float smoothingFactor = 0.05f;

    MastAngle += smoothingFactor * (targetMastAngle - MastAngle);
    BoomAngle += smoothingFactor * (targetBoomAngle - BoomAngle);

    // Sail Physics
    glm::vec3 apparentWind = windStrength * windDirection + forwardVelocity * direction;
    apparentWindSpeed = glm::length(apparentWind);
    glm::vec3 apparentWindDirection = glm::normalize(apparentWind);

    relativeSailAngle = glm::orientedAngle(apparentWindDirection, direction, glm::vec3(0.0f, 0.0f, 1.0f)) + BoomAngle;
    float liftForce = 0.5 * airDensity * maxLiftCoefficient * sin(relativeSailAngle) * sailArea * apparentWindSpeed * apparentWindSpeed;
    glm::vec3 liftDirection = glm::vec3(-apparentWindDirection[1], apparentWindDirection[0], 0.0f);

    float sailDragForce;
    if (abs(relativeSailAngle) < 0.1f)
    {
        sailDragForce = 0.0f; // No drag when sail and wind are aligned
    }
    else
    {
        sailDragForce = 0.5 * airDensity * (minDragCoefficient + (1 - minDragCoefficient) * abs(cos(relativeSailAngle))) * sailArea * apparentWindSpeed * apparentWindSpeed;
    }

    float bodyDragForce = 0.5 * airDensity * bodyDragCoefficient * bodyArea * forwardVelocity * forwardVelocity;

    float resistanceForce = rollCoefficient * mass * g * forwardVelocity * forwardVelocity;
    float F_forward = liftForce * glm::dot(liftDirection, direction);
    float F_backward = sailDragForce * glm::dot(apparentWindDirection, direction) + resistanceForce;

    forwardAcceleration += (F_forward - F_backward) / mass;

    // Apply accelerations
    forwardVelocity += forwardAcceleration * EventHandler::deltaTime;
    steeringAngle += (steeringChange - steeringAngle * steeringSmoothness) * EventHandler::deltaTime;

    // Transform with velocities
    baseTransform *= glm::rotate(glm::mat4(1.0f), glm::radians(steeringAngle * forwardVelocity * EventHandler::deltaTime), glm::vec3(0.0f, 0.0f, -1.0f));
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * EventHandler::deltaTime, 0.0f));
}

void Physics::debug()
{
    Render::debugData.push_back(std::pair("velocity", forwardVelocity));
    Render::debugData.push_back(std::pair("acceleration", forwardAcceleration));
    Render::debugData.push_back(std::pair("apparantWind", apparentWindSpeed));
    Render::debugData.push_back(std::pair("steeringAngle", steeringAngle));
    Render::debugData.push_back(std::pair("angleToWind", glm::degrees(angleToWind)));
    Render::debugData.push_back(std::pair("relativeAngle", glm::degrees(relativeSailAngle)));
}