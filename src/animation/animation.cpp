#include "animation/animation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "physics/physics.h"
#include "event_handler/event_handler.h"
#include "camera/camera.h"

void Animation::updateYachtBones(ModelData &ModelData, float &alpha)
{
    // Abreviations
    Model *model = ModelData.model;
    Physics *physics = ModelData.physics->getReadBuffer();

    // Interpolate between physics ticks
    float steeringAngle = glm::mix(physics->prevSteeringAngle, physics->steeringAngle, alpha);
    float wheelAngle = glm::mix(physics->prevWheelAngle, physics->wheelAngle, alpha);
    float mastAngle = glm::mix(physics->prevMastAngle, physics->MastAngle, alpha);
    float boomAngle = glm::mix(physics->prevBoomAngle, physics->BoomAngle, alpha);
    float sailAngle = glm::mix(physics->prevSailAngle, physics->SailAngle, alpha);

    // Decompose transforms
    glm::vec3 prevPos = glm::vec3(physics->prevBaseTransform[3]);
    glm::vec3 currPos = glm::vec3(physics->baseTransform[3]);

    glm::quat prevRot = glm::quat_cast(physics->prevBaseTransform);
    glm::quat currRot = glm::quat_cast(physics->baseTransform);

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
    model->boneHierarchy["Armature_Boom"]->transform = glm::rotate(glm::rotate(
                                                                       glm::mat4(1.0f), abs(sailAngle - boomAngle) / 2.0f,
                                                                       glm::vec3(1.0f, 0.0f, 0.0f)),
                                                                   boomAngle - mastAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    model->boneHierarchy["Armature_Sail"]->transform = glm::rotate(glm::mat4(1.0f), sailAngle - mastAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    // Push updates to children
    model->updateBoneTransforms();
};