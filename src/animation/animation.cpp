#include "animation/animation.h"

#include <glm/gtc/matrix_transform.hpp>

#include "physics/physics.h"
#include "event_handler/event_handler.h"
#include "camera/camera.h"

void Animation::updateYachtBones(ModelData &ModelData)
{
    // Abreviations
    Model *model = ModelData.model;
    Physics *physics = ModelData.physics[0];

    // Body Transform
    ModelData.u_model = physics->baseTransform;

    // Wheel transforms
    model->boneHierarchy["Armature_Fork"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->steeringAngle * 2), glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(physics->wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-physics->wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Sail setup transform
    model->boneHierarchy["Armature_Mast"]->transform = glm::rotate(glm::mat4(1.0f), physics->MastAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Boom"]->transform = glm::rotate(glm::rotate(
                                                                       glm::mat4(1.0f), abs(physics->SailAngle - physics->BoomAngle) / 2.0f,
                                                                       glm::vec3(1.0f, 0.0f, 0.0f)),
                                                                   physics->BoomAngle - physics->MastAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    model->boneHierarchy["Armature_Sail"]->transform = glm::rotate(glm::mat4(1.0f), physics->SailAngle - physics->MastAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Push updates to children
    model->updateBoneTransforms();

    // If controlled, make camera follow
    if (ModelData.controlled)
    {
        Camera::cameraPosition = (ModelData.u_model * model->boneTransforms[model->boneHierarchy["Armature_Cam"]->index]) * glm::vec4(0, 0, 0, 1);
        Camera::yaw = -atan2(physics->baseTransform[0][1], physics->baseTransform[1][1]);
    }
};