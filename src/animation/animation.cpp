#include <animation/animation.h>

#include "physics/physics.h"
#include "event_handler/event_handler.h"
#include "camera/camera.h"

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

    wheelAngle += physics->forwardVelocity * EventHandler::deltaTime * 100;

    // Body Transform
    model->boneHierarchy["Armature_Body"]->transform = physics->baseTransform;

    // Wheel transforms
    model->boneHierarchy["Armature_Fork"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->steeringAngle * 2), glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Sail setup transform
    model->boneHierarchy["Armature_Mast"]->transform = glm::rotate(glm::mat4(1.0f), physics->MastAngle, glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Boom"]->transform = glm::rotate(glm::rotate(
                                                                       glm::mat4(1.0f), abs(physics->BoomAngle) / 20.0f,
                                                                       glm::vec3(1.0f, 0.0f, 0.0f)),
                                                                   physics->BoomAngle - physics->MastAngle, glm::vec3(0.0f, 0.0f, -1.0f));
    model->boneHierarchy["Armature_Sail"]->transform = glm::rotate(glm::rotate(
                                                                       glm::mat4(1.0f), abs(physics->SailAngle) / 10.0f,
                                                                       glm::vec3(0.0f, -1.0f, 0.0f)),
                                                                   physics->SailAngle - physics->MastAngle / 2, glm::vec3(0.0f, 0.0f, -1.0f));

    model->updateBoneTransforms();

    Camera::cameraPosition = (model->boneTransforms[model->boneHierarchy["Armature_Cam"]->index]) * glm::vec4(0, 0, 0, 1);
    Camera::yaw = atan2(physics->baseTransform[0][1], physics->baseTransform[1][1]) + M_PI;
};