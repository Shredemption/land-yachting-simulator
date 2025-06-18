#include "physics/physics.hpp"

#include "pch.h"

Physics::Physics(const std::string &name)
{
    // Set base transform to 1
    baseTransform = glm::mat4(1.0f);

    // Check which yacht, and apply correct properties
    if (name == "dn-duvel")
    {
        // Max control angles
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        // Sail physics properties
        maxLiftCoefficient = 1.5f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.0f;

        // Body properties
        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 250.0f;
        bodyDragCoefficient = 0.4f;
        bodyArea = 1.2f;

        // Steering properties
        steeringSmoothness = 3.0f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 0.5f;
    }

    else if (name == "red-piper")
    {
        // Max control angles
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        // Sail physics properties
        maxLiftCoefficient = 1.3f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.8f;

        // Body properties
        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 180.0f;
        bodyDragCoefficient = 0.35f;
        bodyArea = 1.0f;

        // Steering properties
        steeringSmoothness = 3.0f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 0.45f;
    }

    else if (name == "blue-piper")
    {
        // Max control angles
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        // Sail physics properties
        maxLiftCoefficient = 1.4f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 6.4f;

        // Body properties
        rollCoefficient = 0.005f;
        rollScaling = 15.0f;
        mass = 180.0f;
        bodyDragCoefficient = 0.35f;
        bodyArea = 1.0f;

        // Steering properties
        steeringSmoothness = 1.5f;
        maxSteeringAngle = 25.0f;
        steeringAttenuation = 1.55f;
    }

    else if (name == "sietske")
    {
        // Max control angles
        maxMastAngle = glm::radians(60.0f);
        maxBoomAngle = glm::radians(90.0f);

        // Sail physics properties
        maxLiftCoefficient = 1.5f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 5.7f;

        // Body properties
        rollCoefficient = 0.004f;
        rollScaling = 20.0f;
        mass = 200.0f;
        bodyDragCoefficient = 0.2f;
        bodyArea = 1.0f;

        // Steering properties
        steeringSmoothness = 2.0f;
        maxSteeringAngle = 22.22f;
        steeringAttenuation = 1.f;
    }

    else
    {
        // Max control angles
        maxMastAngle = glm::radians(40.0f);
        maxBoomAngle = glm::radians(80.0f);

        // Sail physics properties
        maxLiftCoefficient = 1.0f;
        optimalAngle = glm::radians(20.0f);
        minDragCoefficient = 0.1f;
        sailArea = 5.0f;

        // Body properties
        rollCoefficient = 0.005f;
        rollScaling = 20.0f;
        mass = 100.0f;
        bodyDragCoefficient = 0.1f;
        bodyArea = 1.0f;

        // Steering properties
        steeringSmoothness = 1.0f;
        maxSteeringAngle = 30.0f;
        steeringAttenuation = 1.0f;
    }
}

void Physics::reset(const glm::mat4 &u_model)
{
    baseTransform = u_model;
    sailControlFactor = 1.0f;
    MastAngle = 0.0f;
    BoomAngle = 0.0f;
    SailAngle = 0.0f;
    steeringAngle = 0.0f;
    forwardVelocity = 0.0f;
    wheelAngle = 0.0f;
}

void Physics::move(bool &controlled)
{
    // Acceleration from keys
    float forwardAcceleration = 0.0f;
    float steeringChange = 0.0f;

    double tickRate = SettingsManager::settings.physics.tickRate;

    if (InputManager::inputType == InputType::Controller)
    {
        if (controlled)
        {
            if (ControllerManager::state.buttons[GLFW_GAMEPAD_BUTTON_A].held())
            {
                forwardAcceleration += 1.f;
            }

            steeringChange = -ControllerManager::state.sticks[0].x;

            sailControlFactor += -ControllerManager::state.sticks[0].y * tickRate;
        }

        steeringAngle = 0.5 * steeringAngle + 0.5 * steeringChange * maxSteeringAngle;
    }
    else if (controlled)
    {
        {
            if (PhysicsUtil::keyInputs[0])
            {
                sailControlFactor += 1.f * tickRate;
            }
            if (PhysicsUtil::keyInputs[1])
            {
                sailControlFactor -= 0.4f * tickRate;
            }
            if (PhysicsUtil::keyInputs[2])
            {
                steeringChange += steeringSmoothness * maxSteeringAngle;
            }
            if (PhysicsUtil::keyInputs[3])
            {
                steeringChange -= steeringSmoothness * maxSteeringAngle;
            }
            if (PhysicsUtil::keyInputs[4])
            {
                forwardAcceleration += 1.f;
            }
        }

        steeringAngle += (steeringChange - steeringAngle * steeringSmoothness) * tickRate;
    }

    // Clamp sail control
    sailControlFactor = std::clamp(sailControlFactor, 0.2f, 1.0f);

    // Find new angles for sail
    glm::vec3 direction = glm::normalize(glm::vec3(baseTransform[1]));
    float angleToWind = glm::orientedAngle(direction, PhysicsUtil::windSourceDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    // Apply angles to sail setup
    float targetMastAngle = (0.5f + sailControlFactor) / 1.5f * std::clamp(angleToWind, -maxMastAngle, maxMastAngle);
    float targetBoomAngle = sailControlFactor * std::clamp(angleToWind, -maxBoomAngle, maxBoomAngle);

    // Smooth angle transitions
    float smoothingFactor = 0.05f;
    MastAngle += smoothingFactor * (targetMastAngle - MastAngle);
    BoomAngle += smoothingFactor * (targetBoomAngle - BoomAngle);
    SailAngle = BoomAngle * (1 + 0.1 * fabs(sin(angleToWind / 2)));

    // Apparent wind direction
    glm::vec3 apparentWind = -PhysicsUtil::windSourceDirection * PhysicsUtil::windStrength - direction * forwardVelocity;
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
    float dynamicPressure = 0.5f * PhysicsUtil::airDensity * apparentWindSpeed * apparentWindSpeed;
    float localLift = dynamicPressure * sailArea * effectiveCL;
    float localDrag = dynamicPressure * sailArea * effectiveCD;

    float sailLiftForce = localLift * sin(angleToApparentWind);
    float sailDragForce = localDrag * cos(angleToApparentWind);

    float bodyDragForce = 0.5f * PhysicsUtil::airDensity * bodyDragCoefficient * bodyArea * forwardVelocity * forwardVelocity;

    // Rolling Resistance
    float effectiveCr = rollCoefficient * (1 + (forwardVelocity * forwardVelocity) / (rollScaling * rollScaling));
    float rollResistance = effectiveCr * mass * PhysicsUtil::g;

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
    forwardVelocity += forwardAcceleration * tickRate;
    float effectiveSteeringAngle = steeringAngle / (1 + steeringAttenuation * forwardVelocity);

    // Transform with velocities
    baseTransform *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(effectiveSteeringAngle * forwardVelocity * tickRate)), glm::vec3(0.0f, 0.0f, 1.0f));
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * tickRate, 0.0f));
    wheelAngle += forwardVelocity * tickRate * 100;

    // Send values to debug
    if (controlled)
    {
        Render::debugPhysicsData.clear();
        Render::debugPhysicsData.push_back(std::pair("velocity", forwardVelocity));
        Render::debugPhysicsData.push_back(std::pair("acceleration", forwardAcceleration));
        Render::debugPhysicsData.push_back(std::pair("apparantWind", apparentWindSpeed));
        Render::debugPhysicsData.push_back(std::pair("steeringAngle", steeringAngle));
        Render::debugPhysicsData.push_back(std::pair("effectiveSteeringAngle", effectiveSteeringAngle));
        Render::debugPhysicsData.push_back(std::pair("angleToWind", glm::degrees(angleToWind)));
        Render::debugPhysicsData.push_back(std::pair("angleToApparentWind", glm::degrees(angleToApparentWind)));
        Render::debugPhysicsData.push_back(std::pair("relativeAngle", glm::degrees(relativeSailAngle)));
        Render::debugPhysicsData.push_back(std::pair("effectiveCL", effectiveCL));
        Render::debugPhysicsData.push_back(std::pair("effectiveCD", effectiveCD));
    }
}

void Physics::savePrevState()
{
    // Save previous state
    prevBaseTransform = baseTransform;
    prevSteeringAngle = steeringAngle;
    prevWheelAngle = wheelAngle;
    prevMastAngle = MastAngle;
    prevBoomAngle = BoomAngle;
    prevSailAngle = SailAngle;
}

void Physics::copyFrom(const Physics &other)
{
    *this = other;
}