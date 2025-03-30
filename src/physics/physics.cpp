#include <physics/physics.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "event_handler/event_handler.h"
#include "render/render.h"

bool Physics::keyInputs[5];
glm::vec3 Physics::windDirection = glm::vec3(0.0f, -1.0f, 0.0f);
float Physics::windStrength = 10.0f;
float Physics::airDensity = 1.225f;
float Physics::g = 9.81f;

bool Physics::resetState = false;

Physics::Physics(ModelData &ModelData)
{
    baseTransform = glm::mat4(1.0f);

    if (ModelData.model->path.find("dn-duvel") != std::string::npos)
    {
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        maxLiftCoefficient = 1.5f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.0f;

        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 250.0f;
        bodyDragCoefficient = 0.3f;
        bodyArea = 1.2f;

        steeringSmoothness = 3.0f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 0.5f;
    }

    else if (ModelData.model->path.find("red-piper") != std::string::npos)
    {
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        maxLiftCoefficient = 1.3f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.8f;

        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 180.0f;
        bodyDragCoefficient = 0.25f;
        bodyArea = 1.0f;

        steeringSmoothness = 3.0f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 0.45f;
    }

    else if (ModelData.model->path.find("blue-piper") != std::string::npos)
    {
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        maxLiftCoefficient = 1.4f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.4f;

        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 180.0f;
        bodyDragCoefficient = 0.25f;
        bodyArea = 1.0f;

        steeringSmoothness = 1.5f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 1.55f;
    }

    else if (ModelData.model->path.find("sietske") != std::string::npos)
    {
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        maxLiftCoefficient = 1.5f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 5.7f;

        rollCoefficient = 0.004f;
        rollScaling = 20.0f;
        mass = 200.0f;
        bodyDragCoefficient = 0.15f;
        bodyArea = 1.0f;

        steeringSmoothness = 2.0f;
        maxSteeringAngle = 22.22f;
        steeringAttenuation = 1.f;
    }

    else
    {
        maxMastAngle = glm::radians(1.0f);
        maxBoomAngle = glm::radians(1.0f);

        maxLiftCoefficient = 1.0f;
        optimalAngle = glm::radians(1.0f);
        minDragCoefficient = 1.0f;
        sailArea = 1.0f;

        rollCoefficient = 1.0f;
        rollScaling = 1.0f;
        mass = 1.0f;
        bodyDragCoefficient = 1.0f;
        bodyArea = 1.0f;

        steeringSmoothness = 1.0f;
        maxSteeringAngle = 1.0f;
        steeringAttenuation = 1.0f;
    }
}

void Physics::reset()
{
    baseTransform = glm::mat4(1.0f);
    sailControlFactor = 1.0f;
    MastAngle = 0.0f;
    BoomAngle = 0.0f;
    SailAngle = 0.0f;
    steeringAngle = 0.0f;
    forwardVelocity = 0.0f;
    wheelAngle = 0.0f;
}

void Physics::setup(Scene &scene)
{
    for (ModelData &model : scene.structModels)
    {
        if (model.animated)
        {
            model.physics.clear();
            model.physics.push_back(new Physics(model));
            model.physics[0]->reset();
        }
    }
}

void Physics::update(Scene &scene)
{
    for (ModelData &model : scene.structModels)
    {
        if (model.controlled)
        {
            model.physics[0]->move();
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
        forwardAcceleration += 1.f;
    }

    sailControlFactor = std::clamp(sailControlFactor, 0.2f, 1.0f);

    // Find new angles for sail
    glm::vec3 direction = glm::normalize(glm::vec3(baseTransform[1]));
    float angleToWind = glm::orientedAngle(direction, -windDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    float targetMastAngle = (0.5f + sailControlFactor) / 1.5f * std::clamp(angleToWind, -maxMastAngle, maxMastAngle);
    float targetBoomAngle = sailControlFactor * std::clamp(angleToWind, -maxBoomAngle, maxBoomAngle);

    float smoothingFactor = 0.05f;

    MastAngle += smoothingFactor * (targetMastAngle - MastAngle);
    BoomAngle += smoothingFactor * (targetBoomAngle - BoomAngle);
    SailAngle = BoomAngle * (1 + 0.1 * fabs(sin(angleToWind / 2)));

    // Apparent wind direction
    glm::vec3 apparentWind = windDirection * windStrength - direction * forwardVelocity;
    float apparentWindSpeed = glm::length(apparentWind);
    glm::vec3 apparentWindDirection = glm::normalize(apparentWind);

    // Relative sail angle
    float angleToApparentWind = glm::orientedAngle(direction, -apparentWindDirection, glm::vec3(0.0f, 0.0f, 1.0f));
    float relativeSailAngle = angleToApparentWind - SailAngle;
    float absAngle = fabs(relativeSailAngle);

    // Lift and Drag coefficients
    float effectiveCL = (absAngle <= optimalAngle ? maxLiftCoefficient * (relativeSailAngle / optimalAngle) * sin(2.0f * absAngle) / sin(2.0f * optimalAngle) : maxLiftCoefficient * (optimalAngle / absAngle) * (relativeSailAngle < 0 ? -1.0f : 1.0f));
    float effectiveCD = minDragCoefficient + sin(absAngle) * sin(absAngle);

    // Lift and Drag forces
    float dynamicPressure = 0.5f * airDensity * apparentWindSpeed * apparentWindSpeed;
    float localLift = dynamicPressure * sailArea * effectiveCL;
    float localDrag = dynamicPressure * sailArea * effectiveCD;

    float sailLiftForce = localLift * sin(angleToApparentWind);
    float sailDragForce = localDrag * cos(angleToApparentWind);

    float bodyDragForce = 0.5f * airDensity * bodyDragCoefficient * bodyArea * forwardVelocity * forwardVelocity;

    // Rolling Resistance
    float effectiveCr = rollCoefficient * (1 + (forwardVelocity * forwardVelocity) / (rollScaling * rollScaling));
    float rollResistance = effectiveCr * mass * g;

    // Stationary force/acceleration
    const float standstillVelocity = 0.02f;
    float netForce;

    // if stationary
    if (forwardVelocity < standstillVelocity)
    {
        // if propulsion less than rollresistance
        if (sailLiftForce - sailDragForce - bodyDragForce < rollResistance)
        {
            netForce = 0.0f;
            forwardVelocity *= 0.75f;
        }
        // if propulsion greater than rollresistance
        else
        {
            netForce = sailLiftForce - sailDragForce - bodyDragForce - rollResistance;
        }
    }
    // if already moving
    else
    {
        netForce = sailLiftForce - sailDragForce - bodyDragForce - rollResistance;
    }

    forwardAcceleration += netForce / mass;

    // Apply accelerations
    forwardVelocity += forwardAcceleration * EventHandler::deltaTime;
    steeringAngle += (steeringChange - steeringAngle * steeringSmoothness) * EventHandler::deltaTime;
    float effectiveSteeringAngle = steeringAngle / (1 + steeringAttenuation * forwardVelocity);

    // Transform with velocities
    baseTransform *= glm::rotate(glm::mat4(1.0f), glm::radians(effectiveSteeringAngle * forwardVelocity * EventHandler::deltaTime), glm::vec3(0.0f, 0.0f, -1.0f));
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * EventHandler::deltaTime, 0.0f));
    wheelAngle += forwardVelocity * EventHandler::deltaTime * 100;

    // Send values to debug
    Render::debugData.push_back(std::pair("velocity", forwardVelocity));
    Render::debugData.push_back(std::pair("acceleration", forwardAcceleration));
    Render::debugData.push_back(std::pair("apparantWind", apparentWindSpeed));
    Render::debugData.push_back(std::pair("steeringAngle", steeringAngle));
    Render::debugData.push_back(std::pair("effectiveSteeringAngle", effectiveSteeringAngle));
    Render::debugData.push_back(std::pair("angleToWind", glm::degrees(angleToWind)));
    Render::debugData.push_back(std::pair("angleToApparentWind", glm::degrees(angleToApparentWind)));
    Render::debugData.push_back(std::pair("relativeAngle", glm::degrees(relativeSailAngle)));
    Render::debugData.push_back(std::pair("effectiveCL", effectiveCL));
    Render::debugData.push_back(std::pair("effectiveCD", effectiveCD));
}

void Physics::switchControlledYacht(Scene &scene)
{
    std::string current;

    for (auto &model : scene.structModels)
    {
        if (model.controlled && model.physics[0]->forwardVelocity <= 0.01f)
        {
            current = model.model->name;
            model.controlled = false;
        }
    }

    int currentId = find(scene.loadedYachts.begin(), scene.loadedYachts.end(), current) - scene.loadedYachts.begin();
    int newId = (currentId + 1) % scene.loadedYachts.size();

    for (auto &model : scene.structModels)
    {
        if (model.model->name == scene.loadedYachts[newId])
        {
            model.controlled = true;
        }
    }
}