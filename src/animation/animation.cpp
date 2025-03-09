#include <animation/animation.h>

#include <glm/gtc/matrix_transform.hpp>

float wheelAngle;

void Animation::updateBones(Scene &scene)
{
    for (auto ModelData : scene.structModels)
    {
        if (ModelData.animation == "yacht_generated")
        {
            generateYachtBones(ModelData.model);
        };
    };
}

void Animation::generateYachtBones(Model *model)
{
    wheelAngle += 2.5f;

    if (model->path.find("duvel/duvel") != std::string::npos) {
        std::string yacht = "duvel";
    };

    model->boneHierarchy["Armature_Wheel_Front"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Left"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model->boneHierarchy["Armature_Wheel_Right"]->transform = glm::rotate(glm::mat4(1.0f), glm::radians(-wheelAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    model->updateBoneTransforms();
};