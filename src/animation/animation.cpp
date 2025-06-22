#include "animation/animation.hpp"

#include "pch.h"

#include "model/bone.h"

void Animation::update(ModelData &ModelData, const float &alpha)
{
    switch (ModelData.model->modelType)
    {
    default:
        updateGeneric(ModelData, alpha);
        break;
    }
}

void Animation::update(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones)
{
    switch (ModelData.model->modelType)
    {
    case ModelType::Yacht:
        updateYachtBones(ModelData, alpha, targetBones);
        break;
    default:
        updateGeneric(ModelData, alpha);
        break;
    }
}

void Animation::updateGeneric(ModelData &ModelData, const float &alpha)
{
    // Abreviations
    Model *model = ModelData.model;
    Physics *physics = ModelData.physics->getReadBuffer();

    // Decompose transforms
    glm::vec3 prevPos = physics->base.prevPos;
    glm::vec3 currPos = physics->base.pos;

    glm::quat prevRot = physics->base.prevRot;
    glm::quat currRot = physics->base.rot;

    // Interpolate
    glm::vec3 interpPos = glm::mix(prevPos, currPos, alpha);
    glm::quat interpRot = glm::slerp(prevRot, currRot, alpha);

    // Reconstruct matrix
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), interpPos) * glm::toMat4(interpRot);

    // Body Transform
    ModelData.u_model = transform;
}

void Animation::updateYachtBones(ModelData &ModelData, const float &alpha, std::vector<glm::mat4> &targetBones)
{
    // Abreviations
    Model *model = ModelData.model;
    Physics *physics = ModelData.physics->getReadBuffer();

    BodyVariables *body = nullptr;
    SailVariables *sail = nullptr;
    DrivingVariables *driving = nullptr;
    physics->getVariablePointers(body, sail, driving);

    // Interpolate between physics ticks
    float steeringAngle = glm::mix(driving->prevSteeringAngle, driving->steeringAngle, alpha);
    float wheelAngle = glm::mix(driving->prevWheelAngle, driving->wheelAngle, alpha);
    float mastAngle = glm::mix(sail->prevMastAngle, sail->MastAngle, alpha);
    float boomAngle = glm::mix(sail->prevBoomAngle, sail->BoomAngle, alpha);
    float sailAngle = glm::mix(sail->prevSailAngle, sail->SailAngle, alpha);

    // Decompose transforms
    glm::vec3 prevPos = physics->base.prevPos;
    glm::vec3 currPos = physics->base.pos;

    glm::quat prevRot = physics->base.prevRot;
    glm::quat currRot = physics->base.rot;

    // Interpolate
    glm::vec3 interpPos = glm::mix(prevPos, currPos, alpha);
    glm::quat interpRot = glm::slerp(prevRot, currRot, alpha);

    // Reconstruct matrix
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), interpPos) * glm::toMat4(interpRot);

    // Body Transform
    ModelData.u_model = transform;

    // Wheel transforms
    model->boneHierarchy["Armature_Fork"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(steeringAngle * 2), glm::vec3(0.0f, -1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Sail setup transform
    model->boneHierarchy["Armature_Mast"]->transform = glm::rotate(glm::mat4(1.0f), mastAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Boom"]->transform = glm::rotate(glm::mat4(1.0f), boomAngle - mastAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    model->boneHierarchy["Armature_Sail"]->transform = glm::rotate(glm::mat4(1.0f), sailAngle - mastAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Push updates to children
    model->updateBoneTransforms(targetBones);
};