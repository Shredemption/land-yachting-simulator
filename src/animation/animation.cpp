#include <animation/animation.h>

#include <physics/physics.h>
#include <event_handler/event_handler.h>

#include <glm/gtc/matrix_transform.hpp>

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

    model->boneHierarchy["Armature_Fork"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->steeringAngle * 2), glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    model->boneHierarchy["Armature_Body"]->transform = physics->baseTransform;

    model->updateBoneTransforms();
};