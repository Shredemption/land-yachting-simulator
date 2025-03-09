#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

#include <scene/scene.h>

class Physics
{
public:
    Physics();

    static glm::vec3 windDirection;
    static void setup(Scene &scene);
    static void update(Scene &scene);

    static bool keyInputs[4];

    glm::mat4 baseTransform;
    float steeringAngle;
    float steeringChange;
    float sailAngle;
    float forwardVelocity;
    float forwardAcceleration;
    
private:
    void move();
};

#endif