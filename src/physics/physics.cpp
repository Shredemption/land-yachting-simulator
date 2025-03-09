#include <physics/physics.h>

#include <event_handler/event_handler.h>

#include <glm/gtc/matrix_transform.hpp>

bool Physics::keyInputs[4];
glm::vec3 Physics::windDirection = glm::vec3(1.0f, 0.0f, 0.0f);

Physics::Physics()
{
    baseTransform = glm::mat4(1.0f);
    steeringAngle = 0.0f;
    sailAngle = 0.0f;
    forwardVelocity = 0.0f;
    forwardAcceleration = 0.0f;
}

void Physics::setup(Scene &scene)
{
    for (int i = 0; i < scene.structModels.size(); i++)
    {
        if (scene.structModels[i].type == "yacht_controlled")
        {
            scene.structModels[i].physics.push_back(new Physics());
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
    forwardAcceleration = 0.0f;

    if (keyInputs[0])
    {
        forwardAcceleration += 0.5f;
    }
    if (keyInputs[1])
    {
        forwardAcceleration -= 0.5f;
    }

    forwardVelocity += forwardAcceleration * EventHandler::deltaTime;

    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, forwardVelocity * EventHandler::deltaTime, 0.0f));
}