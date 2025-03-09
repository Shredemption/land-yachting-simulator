#include <animation/animation.h>

#include <physics/physics.h>
#include <event_handler/event_handler.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

float wheelAngle;

void Animation::updateBones(Scene &scene)
{
    for (auto ModelData : scene.structModels)
    {
        if (ModelData.type == "yacht_controlled")
        {
            generateYachtBones(ModelData);
        };
    };
}

void Animation::generateYachtBones(ModelData &ModelData)
{

    Model *model = ModelData.model;
    Physics *physics = ModelData.physics[0];

    if (model->path.find("duvel/duvel") != std::string::npos)
    {
        std::string yacht = "duvel";
    };

    wheelAngle += physics->forwardVelocity * EventHandler::deltaTime * 100;

    glm::vec3 direction = glm::vec3(physics->baseTransform[0]);

    float angleToWind = glm::orientedAngle(direction, physics->windDirection, glm::vec3(0, 0, 1));
    std::cout << angleToWind << std::endl;

    model->boneHierarchy["Armature_Fork"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->steeringAngle * 2), glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Mast"]->transform = glm::rotate(glm::mat4(1.0f), angleToWind, glm::vec3(0.0f, -1.0f, 0.0f));

    model->boneHierarchy["Armature_Body"]->transform = physics->baseTransform;

    model->updateBoneTransforms();
};