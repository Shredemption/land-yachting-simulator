#include <physics/physics.h>

#include <glm/gtc/matrix_transform.hpp>

Physics::Physics()
{
    baseTransform = glm::mat4(1.0f);
    steeringAngle = 0.0f;
    sailAngle = 0.0f;
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
    baseTransform *= glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.0f, 0.0f));
}