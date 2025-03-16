#include <physics/physics.h>

#include "event_handler/event_handler.h"
#include "render/render.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

bool Physics::keyInputs[5];
glm::vec3 Physics::windDirection = glm::vec3(0.0f, -1.0f, 0.0f);
float Physics::windStrength = 10;
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

        maxLiftCoefficient = 1.4;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1;
        sailArea = 6;

        rollCoefficient = 0.01;
        mass = 250;
        bodyDragCoefficient = 0.3;
        bodyArea = 1.0;

        steeringSmoothness = 3.0;
        maxSteeringAngle = 10;
    }
}

void Physics::reset()
{
    baseTransform = glm::mat4(1.0f);
    sailControlFactor = 1.0f;
    MastAngle = 0.0f;
    BoomAngle = 0.0f;
    steeringAngle = 0.0f;
    forwardVelocity = 0.0f;
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
    float forwardAcceleration = 0.0f;
    float steeringChange = 0.0f;

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

    sailControlFactor = std::clamp(sailControlFactor, 0.2f, 1.0f);

    // Find new angles for sail
    glm::vec3 direction = glm::normalize(glm::vec3(baseTransform[1]));
    float angleToWind = glm::orientedAngle(direction, -windDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    float targetMastAngle = sailControlFactor * std::clamp(angleToWind, -maxMastAngle, maxMastAngle);
    float targetBoomAngle = sailControlFactor * std::clamp(angleToWind, -maxBoomAngle + maxMastAngle, maxBoomAngle - maxMastAngle);

    float smoothingFactor = 0.05f;

    MastAngle += smoothingFactor * (targetMastAngle - MastAngle);
    BoomAngle += smoothingFactor * (targetBoomAngle - BoomAngle);
    SailAngle = BoomAngle * (1 + 0.1 * sin(angleToWind / 2));

    // Apparent wind direction
    glm::vec3 apparentWind = windDirection * windStrength - direction * forwardVelocity;
    float apparentWindSpeed = glm::length(apparentWind);
    glm::vec3 apparentWindDirection = glm::normalize(apparentWind);

    // Relative sail angle
    float angleToApparentWind = glm::orientedAngle(direction, -apparentWindDirection, glm::vec3(0.0f, 0.0f, 1.0f));
    float relativeSailAngle = angleToApparentWind + SailAngle;
    float absAngle = fabs(relativeSailAngle);

    // Lift and Drag coefficients
    float effectiveCL = (absAngle <= optimalAngle ? maxLiftCoefficient * (relativeSailAngle / optimalAngle) : maxLiftCoefficient * (optimalAngle / absAngle) * (relativeSailAngle < 0 ? -1.0f : 1.0f));
    float effectiveCD = minDragCoefficient + 1.0f * sin(absAngle) * sin(absAngle);

    // Lift and Drag forces
    float dynamicPressure = 0.5f * airDensity * apparentWindSpeed * apparentWindSpeed;
    glm::vec2 F_local_sail = dynamicPressure * sailArea * glm::vec2(effectiveCL, -effectiveCD);

    float F_lateral = F_local_sail.x * cos(angleToApparentWind) - F_local_sail.y * sin(angleToApparentWind); // lateral force
    float F_forward = F_local_sail.x * sin(angleToApparentWind) + F_local_sail.y * cos(angleToApparentWind); // forward thrust

    float bodyDragForce = 0.5f * airDensity * bodyDragCoefficient * bodyArea * forwardVelocity * forwardVelocity;

    // Rolling Resistance
    float effectiveCr = rollCoefficient * (1 + (forwardVelocity * forwardVelocity) / 350.0f);
    float rollResistance = effectiveCr * mass * g;

    // Stationary force/acceleration
    const float standstillVelocity = 0.02f;
    float F_net;

    // if stationary
    if (forwardVelocity < standstillVelocity)
    {
        // if propulsion less than rollresistance
        if (F_forward - bodyDragForce < rollResistance)
        {
            F_net = 0;
        }
        // if propulsion greater than rollresistance
        else
        {
            F_net = F_forward - bodyDragForce - rollResistance;
        }
    }
    // if already moving
    else
    {
        F_net = F_forward - bodyDragForce - rollResistance;
    }

    forwardAcceleration += F_net / mass;

    // Apply accelerations
    forwardVelocity += forwardAcceleration * EventHandler::deltaTime;
    steeringAngle += (steeringChange - steeringAngle * steeringSmoothness) * EventHandler::deltaTime;

    // Transform with velocities
    baseTransform *= glm::rotate(glm::mat4(1.0f), glm::radians(steeringAngle * forwardVelocity * EventHandler::deltaTime), glm::vec3(0.0f, 0.0f, -1.0f));
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * EventHandler::deltaTime, 0.0f));

    // Send values to debug
    Render::debugData.push_back(std::pair("velocity", forwardVelocity));
    Render::debugData.push_back(std::pair("acceleration", forwardAcceleration));
    Render::debugData.push_back(std::pair("apparantWind", apparentWindSpeed));
    Render::debugData.push_back(std::pair("steeringAngle", steeringAngle));
    Render::debugData.push_back(std::pair("angleToWind", glm::degrees(angleToWind)));
    Render::debugData.push_back(std::pair("angleToApparentWind", glm::degrees(angleToApparentWind)));
    Render::debugData.push_back(std::pair("relativeAngle", glm::degrees(relativeSailAngle)));
    Render::debugData.push_back(std::pair("effectiveCL", effectiveCL));
    Render::debugData.push_back(std::pair("effectiveCD", effectiveCD));
}