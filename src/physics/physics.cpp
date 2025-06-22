#include "physics/physics.hpp"

#include "pch.h"

Physics::Physics(const ModelData &model)
{
    for (auto type : model.physicsTypes)
    {
        switch (type)
        {
        case PhysicsType::Body:
            bodyVariables.emplace();
            break;
        case PhysicsType::Driving:
            drivingVariables.emplace();
            break;
        case PhysicsType::Sail:
            sailVariables.emplace();
            break;
        case PhysicsType::Gravity:
            applyGravity = true;
            break;
        case PhysicsType::Collision:
            collisionVariables.emplace();
            break;
        }
    }

    BodyVariables *body = nullptr;
    SailVariables *sail = nullptr;
    DrivingVariables *driving = nullptr;
    getVariablePointers(body, sail, driving);

    std::string name = model.model->name;

    switch (model.model->modelType)
    {
    case ModelType::Yacht:
    {
        if (name == "dn-duvel")
        {
            // Max control angles
            sail->maxMastAngle = glm::radians(60.0f);
            sail->maxBoomAngle = glm::radians(90.0f);

            // Sail physics properties
            sail->maxLiftCoefficient = 1.5f;
            sail->optimalAngle = glm::radians(20.0f);
            sail->minDragCoefficient = 0.1f;
            sail->area = 6.0f;

            // Body properties
            driving->rollCoefficient = 0.01f;
            driving->rollScaling = 15.0f;
            base.mass = 250.0f;
            body->dragCoefficient = 0.4f;
            body->area = 1.2f;

            // Steering properties
            driving->steeringSmoothness = 3.0f;
            driving->maxSteeringAngle = 25.0f;
            driving->steeringAttenuation = 0.5f;
        }

        else if (name == "red-piper")
        {
            // Max control angles
            sail->maxMastAngle = glm::radians(60.0f);
            sail->maxBoomAngle = glm::radians(90.0f);

            // Sail physics properties
            sail->maxLiftCoefficient = 1.3f;
            sail->optimalAngle = glm::radians(20.0f);
            sail->minDragCoefficient = 0.1f;
            sail->area = 6.8f;

            // Body properties
            driving->rollCoefficient = 0.01f;
            driving->rollScaling = 15.0f;
            base.mass = 180.0f;
            body->dragCoefficient = 0.35f;
            body->area = 1.0f;

            // Steering properties
            driving->steeringSmoothness = 3.0f;
            driving->maxSteeringAngle = 25.0f;
            driving->steeringAttenuation = 0.45f;
        }

        else if (name == "blue-piper")
        {
            // Max control angles
            sail->maxMastAngle = glm::radians(60.0f);
            sail->maxBoomAngle = glm::radians(90.0f);

            // Sail physics properties
            sail->maxLiftCoefficient = 1.4f;
            sail->optimalAngle = glm::radians(20.0f);
            sail->minDragCoefficient = 0.1f;
            sail->area = 6.4f;

            // Body properties
            driving->rollCoefficient = 0.01f;
            driving->rollScaling = 15.0f;
            base.mass = 180.0f;
            body->dragCoefficient = 0.35f;
            body->area = 1.0f;

            // Steering properties
            driving->steeringSmoothness = 1.5f;
            driving->maxSteeringAngle = 25.0f;
            driving->steeringAttenuation = 1.55f;
        }

        else if (name == "sietske")
        {
            // Max control angles
            sail->maxMastAngle = glm::radians(60.0f);
            sail->maxBoomAngle = glm::radians(90.0f);

            // Sail physics properties
            sail->maxLiftCoefficient = 1.5f;
            sail->optimalAngle = glm::radians(20.0f);
            sail->minDragCoefficient = 0.1f;
            sail->area = 5.7f;

            // Body properties
            driving->rollCoefficient = 0.008f;
            driving->rollScaling = 20.0f;
            base.mass = 200.0f;
            body->dragCoefficient = 0.2f;
            body->area = 1.0f;

            // Steering properties
            driving->steeringSmoothness = 2.0f;
            driving->maxSteeringAngle = 22.22f;
            driving->steeringAttenuation = 1.f;
        }

        else
        {
            // Max control angles
            sail->maxMastAngle = glm::radians(40.0f);
            sail->maxBoomAngle = glm::radians(80.0f);

            // Sail physics properties
            sail->maxLiftCoefficient = 1.0f;
            sail->optimalAngle = glm::radians(20.0f);
            sail->minDragCoefficient = 0.1f;
            sail->area = 5.0f;

            // Body properties
            driving->rollCoefficient = 0.01f;
            driving->rollScaling = 20.0f;
            base.mass = 100.0f;
            body->dragCoefficient = 0.1f;
            body->area = 1.0f;

            // Steering properties
            driving->steeringSmoothness = 1.0f;
            driving->maxSteeringAngle = 30.0f;
            driving->steeringAttenuation = 1.0f;
        }
        break;
    }
    }
}

void Physics::reset(const glm::mat4 &u_model)
{
    BodyVariables *body = nullptr;
    SailVariables *sail = nullptr;
    DrivingVariables *driving = nullptr;
    getVariablePointers(body, sail, driving);

    base.pos = glm::vec3(u_model[3]);
    base.vel = glm::vec3(0.0f);

    base.rot = glm::quat_cast(glm::mat3(u_model));

    if (sailVariables)
    {
        sail->controlFactor = 1.0f;
        sail->MastAngle = 0.0f;
        sail->BoomAngle = 0.0f;
        sail->SailAngle = 0.0f;
    }

    if (drivingVariables)
    {
        driving->steeringAngle = 0.0f;
        driving->wheelAngle = 0.0f;
    }
}

void Physics::savePrevState()
{
    base.prevPos = base.pos;
    base.prevRot = base.rot;

    if (sailVariables)
    {
        SailVariables *sail = &sailVariables.value();
        sail->prevMastAngle = sail->MastAngle;
        sail->prevBoomAngle = sail->BoomAngle;
        sail->prevSailAngle = sail->SailAngle;
    }

    if (drivingVariables)
    {
        DrivingVariables *driving = &drivingVariables.value();
        driving->prevSteeringAngle = driving->steeringAngle;
        driving->prevWheelAngle = driving->wheelAngle;
    }
}

void Physics::copyFrom(const Physics &other)
{
    *this = other;
}

void Physics::updateInputs(bool controlled)
{
    BodyVariables *body = nullptr;
    SailVariables *sail = nullptr;
    DrivingVariables *driving = nullptr;
    getVariablePointers(body, sail, driving);

    float tickTime = (1 / SettingsManager::settings.physics.tickRate);

    if (InputManager::inputType == InputType::Controller)
    {
        if (ControllerManager::state.buttons[GLFW_GAMEPAD_BUTTON_A].held())
        {
            base.acc += base.rot * glm::vec3(0, 1, 0);
        }

        driving->steeringChange = -ControllerManager::state.sticks[0].x;

        sail->controlFactor += -ControllerManager::state.sticks[0].y * tickTime;
    }
    else
    {
        {
            if (PhysicsUtil::keyInputs[0])
            {
                sail->controlFactor += 1.f * tickTime;
            }
            if (PhysicsUtil::keyInputs[1])
            {
                sail->controlFactor -= 0.4f * tickTime;
            }
            if (PhysicsUtil::keyInputs[2])
            {
                driving->steeringChange += driving->steeringSmoothness * driving->maxSteeringAngle;
            }
            if (PhysicsUtil::keyInputs[3])
            {
                driving->steeringChange -= driving->steeringSmoothness * driving->maxSteeringAngle;
            }
            if (PhysicsUtil::keyInputs[4])
            {
                base.acc += base.rot * glm::vec3(0, 1, 0);
            }
        }
    }

    sail->controlFactor = std::clamp(sail->controlFactor, 0.2f, 1.0f);
}

void Physics::updateSail(bool debug)
{
    SailVariables *sail = &sailVariables.value();

    // Find new angles for sail
    glm::vec3 direction = base.rot * glm::vec3(0, 1, 0);
    float angleToWind = glm::orientedAngle(direction, PhysicsUtil::windSourceDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    // Apply angles to sail setup
    float targetMastAngle = (0.5f + sail->controlFactor) / 1.5f * std::clamp(angleToWind, -sail->maxMastAngle, sail->maxMastAngle);
    float targetBoomAngle = sail->controlFactor * std::clamp(angleToWind, -sail->maxBoomAngle, sail->maxBoomAngle);

    // Smooth angle transitions
    float smoothingFactor = 0.05f;
    sail->MastAngle += smoothingFactor * (targetMastAngle - sail->MastAngle);
    sail->BoomAngle += smoothingFactor * (targetBoomAngle - sail->BoomAngle);
    sail->SailAngle = sail->BoomAngle * (1 + 0.1 * fabs(sin(angleToWind / 2)));

    // Apparent wind direction
    glm::vec3 apparentWind = -PhysicsUtil::windSourceDirection * PhysicsUtil::windStrength - direction * glm::length(base.vel);
    float apparentWindSpeed = glm::length(apparentWind);
    glm::vec3 apparentWindDirection = glm::normalize(apparentWind);

    // Relative sail angle
    float angleToApparentWind = glm::orientedAngle(direction, -apparentWindDirection, glm::vec3(0.0f, 0.0f, 1.0f));
    float relativeSailAngle = angleToApparentWind - sail->SailAngle;
    float absAngle = fabs(relativeSailAngle);

    // Lift and Drag coefficients
    float effectiveCL = (absAngle <= sail->optimalAngle ? sail->maxLiftCoefficient * (relativeSailAngle / sail->optimalAngle) * sin(2.0f * absAngle) / sin(2.0f * sail->optimalAngle) : sail->maxLiftCoefficient * (sail->optimalAngle / absAngle) * (relativeSailAngle < 0 ? -1.0f : 1.0f));
    float effectiveCD = sail->minDragCoefficient + sin(absAngle) * sin(absAngle);

    // Lift and Drag forces
    float dynamicPressure = 0.5f * PhysicsUtil::airDensity * apparentWindSpeed * apparentWindSpeed;
    float liftMagnitude = dynamicPressure * sail->area * effectiveCL;
    float dragMagnitude = dynamicPressure * sail->area * effectiveCD;

    glm::vec3 dragDir = -apparentWindDirection;
    glm::vec3 liftDir = glm::cross(dragDir, glm::vec3(0, 0, 1));
    if (glm::length(liftDir) > 0.0f)
        liftDir = glm::normalize(liftDir);
    else
        liftDir = glm::vec3(0);

    glm::vec3 sailLiftForce = liftMagnitude * liftDir;
    glm::vec3 sailDragForce = dragMagnitude * dragDir;

    base.netForce += sailLiftForce + sailDragForce;

    debugForces.push_back({base.pos, sailLiftForce, "Lift"});
    debugForces.push_back({base.pos, sailDragForce, "Drag"});

    if (debug)
    {
        Render::debugPhysicsData.push_back(std::pair("apparantWind", apparentWindSpeed));
        Render::debugPhysicsData.push_back(std::pair("angleToWind", glm::degrees(angleToWind)));
        Render::debugPhysicsData.push_back(std::pair("angleToApparentWind", glm::degrees(angleToApparentWind)));
        Render::debugPhysicsData.push_back(std::pair("relativeAngle", glm::degrees(relativeSailAngle)));
        Render::debugPhysicsData.push_back(std::pair("effectiveCL", effectiveCL));
        Render::debugPhysicsData.push_back(std::pair("effectiveCD", effectiveCD));
    }
}

void Physics::updateDriving(bool debug)
{
    DrivingVariables *driving = &drivingVariables.value();

    float tickTime = (1 / SettingsManager::settings.physics.tickRate);

    // Rolling Resistance
    if (glm::length(base.vel) > 1e-4f)
    {
        glm::vec3 rollDir = -glm::normalize(base.vel);
        float effectiveCr = driving->rollCoefficient * (1 + glm::dot(base.vel, base.vel) / (driving->rollScaling * driving->rollScaling));
        float rollResistance = effectiveCr * base.mass * PhysicsUtil::g;
        base.netForce += rollResistance * rollDir;
    }

    driving->wheelAngle += glm::length(base.vel) * 100 / SettingsManager::settings.physics.tickRate;

    float effectiveSteeringAngle = driving->steeringAngle / (1 + driving->steeringAttenuation * glm::length(base.vel));

    glm::quat deltaRot = glm::angleAxis(glm::radians(effectiveSteeringAngle * glm::length(base.vel) * tickTime), glm::vec3(0, 0, 1));

    base.rot = normalize(deltaRot * base.rot);

    if (debug)
    {
        Render::debugPhysicsData.push_back(std::pair("steeringAngle", driving->steeringAngle));
        Render::debugPhysicsData.push_back(std::pair("effectiveSteeringAngle", effectiveSteeringAngle));
    }
}

void Physics::updateBody(bool debug)
{
    BodyVariables *body = &bodyVariables.value();

    if (glm::length(base.vel) > 1e-4f)
    {
        glm::vec3 dragDir = -glm::normalize(base.vel);
        float bodyDragForce = 0.5f * PhysicsUtil::airDensity * body->dragCoefficient * body->area * glm::dot(base.vel, base.vel);
        base.netForce += bodyDragForce * dragDir;
    }
}

void Physics::updateGravity(bool debug)
{
    if (!onGround)
        base.acc += glm::vec3(0, 0, -PhysicsUtil::g);
}

void Physics::checkCollisions(ModelData &modelData)
{
    float tickTime = (1 / SettingsManager::settings.physics.tickRate);

    float groundPos = 0.0f;

    glm::mat4 nextModel = modelData.u_model;
    nextModel[3] = glm::vec4(base.pos, 1.0f);

    float penetration = -1e20f;

    for (auto &meshVariant : modelData.model->hitboxMeshes.value())
    {
        auto *mesh = std::get_if<Mesh<VertexHitbox>>(&meshVariant);
        if (!mesh)
            continue;
        glm::vec3 furthest = mesh->furthestInDirection(glm::vec3(0, 0, -1), nextModel);
        penetration = std::max(groundPos - furthest.z, penetration);
    }

    if (penetration > 0.0f)
    {
        base.pos.z += penetration;
        base.vel.z = 0.0f;
        base.acc.z = 0.0f;
        onGround = true;
    }
}

void Physics::update(ModelData &modelData)
{
    debugForces.clear();

    BodyVariables *body = nullptr;
    SailVariables *sail = nullptr;
    DrivingVariables *driving = nullptr;
    getVariablePointers(body, sail, driving);

    float tickTime = (1 / SettingsManager::settings.physics.tickRate);

    // Reset temp variables
    base.acc = glm::vec3(0.0f);
    base.netForce = glm::vec3(0.0f);

    if (drivingVariables)
        driving->steeringChange = 0.0f;

    if (modelData.controlled)
    {
        Render::debugPhysicsData.clear();
        updateInputs(modelData.controlled);
    }

    if (drivingVariables)
    {
        if (InputManager::inputType == InputType::Controller)
            driving->steeringAngle = 0.5 * driving->steeringAngle + 0.5 * driving->steeringChange * driving->maxSteeringAngle;
        else
            driving->steeringAngle += (driving->steeringChange - driving->steeringAngle * driving->steeringSmoothness) * tickTime;
    }

    if (sailVariables)
        updateSail(modelData.controlled);

    if (drivingVariables)
        updateDriving(modelData.controlled);

    if (bodyVariables)
        updateBody(modelData.controlled);

    if (applyGravity)
        updateGravity(modelData.controlled);

    // Stationary force/acceleration
    const float standstillVelocity = 0.02f;

    // if stationary
    if ((glm::length(base.vel) < standstillVelocity) && (glm::dot(base.netForce, base.vel) <= 0.0f))
    {
        base.netForce = glm::vec3(0.0f);
        base.vel *= 0.2f;
    }

    if (drivingVariables)
    {
        glm::vec3 forward = base.rot * glm::vec3(0, 1, 0);

        glm::vec3 forwardForce = glm::dot(base.netForce, forward) * forward;
        base.acc += forwardForce / base.mass;
        base.vel += base.acc * tickTime;
        glm::vec3 lateral = base.vel - glm::dot(base.vel, forward) * forward;
        base.vel -= lateral * 0.9f;

        base.pos += base.vel * tickTime;
    }
    else
    {
        base.acc += base.netForce / base.mass;
        base.vel += base.acc * tickTime;
        base.pos += base.vel * tickTime;
    }

    checkCollisions(modelData);

    if (modelData.controlled)
    {
        Render::debugPhysicsData.push_back(std::pair("velocity", glm::length(base.vel)));
        Render::debugPhysicsData.push_back(std::pair("acceleration", glm::length(base.acc)));
    }
}