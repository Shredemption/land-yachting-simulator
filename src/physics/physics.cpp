#include <physics/physics.h>

#include <event_handler/event_handler.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

bool Physics::keyInputs[4];
glm::vec3 Physics::windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
float Physics::windStrength = 5;
float Physics::airDensity = 1.2;
float Physics::g = 9.81;

Physics::Physics(ModelData &ModelData)
{
    baseTransform = glm::mat4(1.0f);

    if (ModelData.model->path.find("duvel/duvel") != std::string::npos)
    {
        maxMastAngle = glm::radians(30.0f);
        maxBoomAngle = glm::radians(90.0f);
        maxLiftCoefficient = 1.2;
        minDragCoefficient = 0.1;
        sailArea = 5;
        rollCoefficient = 0.05;
        mass = 50;
        bodyDragCoefficient = 0.5;
        bodyArea = 1;
    }
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
    // Acceleration from keys
    forwardAcceleration = 0.0f;
    steeringChange = 0.0f;

    if (keyInputs[0])
    {
        forwardAcceleration += 2.f;
    }
    if (keyInputs[1])
    {
        forwardAcceleration -= 2.f;
    }
    if (keyInputs[2])
    {
        steeringChange += 10.f;
    }
    if (keyInputs[3])
    {
        steeringChange -= 10.f;
    }

    // Find new angles for sail
    glm::vec3 direction = glm::normalize(glm::vec3(baseTransform[0]));
    angleToWind = glm::orientedAngle(direction, windDirection, glm::vec3(0.0f, 0.0f, 1.0f));

    targetMastAngle = std::clamp(angleToWind, -maxMastAngle, maxMastAngle);
    targetBoomAngle = std::clamp(angleToWind, -maxBoomAngle + maxMastAngle, maxBoomAngle - maxMastAngle);

    float smoothingFactor = 0.05f;

    MastAngle += smoothingFactor * (targetBoomAngle - MastAngle);
    BoomAngle += smoothingFactor * (targetBoomAngle - BoomAngle);

    // Sail Physics
    glm::vec3 apparentWind = windStrength * windDirection - forwardVelocity * direction;
    float apparentWindSpeed = glm::length(apparentWind);
    glm::vec3 apparentWindDirection = glm::normalize(apparentWind);

    float relativeSailAngle = glm::orientedAngle(apparentWindDirection, direction, glm::vec3(0, 0, 1)) + BoomAngle;
    float liftForce = 0.5 * airDensity * maxLiftCoefficient * sin(2 * relativeSailAngle) * sailArea * apparentWindSpeed * apparentWindSpeed;
    glm::vec3 liftDirection = glm::vec3(-apparentWindDirection[1], apparentWindDirection[0], 0.0f);

    float sailDragForce;
    if (abs(relativeSailAngle) < 0.1f) {
        sailDragForce = 0.0f; // No drag when sail and wind are aligned
    } else {
        sailDragForce = 0.5 * airDensity * (minDragCoefficient + (1 - minDragCoefficient) * abs(cos(relativeSailAngle))) * sailArea * apparentWindSpeed * apparentWindSpeed;
    }

    float bodyDragForce = 0.5 * airDensity * bodyDragCoefficient * bodyArea * forwardVelocity * forwardVelocity;

    float resistanceForce = rollCoefficient * mass * g * forwardVelocity;
    float F_forward = liftForce * glm::dot(liftDirection, direction);
    float F_backward = sailDragForce * glm::dot(apparentWindDirection, direction) + resistanceForce;

    forwardAcceleration += (F_forward - F_backward) / mass;

    // Apply accelerations
    forwardVelocity += forwardAcceleration * EventHandler::deltaTime;
    steeringAngle += steeringChange * EventHandler::deltaTime;

    // Transform with velocities
    baseTransform *= glm::rotate(glm::mat4(1.0f), glm::radians(steeringAngle * forwardVelocity * EventHandler::deltaTime), glm::vec3(0.0f, 0.0f, -1.0f));
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * EventHandler::deltaTime, 0.0f));
}