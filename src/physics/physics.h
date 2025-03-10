#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

#include <scene/scene.h>

class Physics
{
public:
    Physics(ModelData &ModelData);

    static glm::vec3 windDirection;
    static void setup(Scene &scene);
    static void update(Scene &scene);

    static bool keyInputs[4];

    glm::mat4 baseTransform;
    float steeringAngle = 0.0f;
    float steeringChange = 0.0f;
    float forwardVelocity = 0.0f;
    float forwardAcceleration = 0.0f;

    float maxMastAngle;
    float maxBoomAngle;
    float MastAngle = 0.0f;
    float targetMastAngle = 0.0f;
    float BoomAngle = 0.0f;
    float targetBoomAngle = 0.0f;
    float angleToWind = 0.0f;
    
private:
    void move();
};

#endif