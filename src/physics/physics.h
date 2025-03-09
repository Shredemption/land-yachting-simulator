#ifndef PHYSICS_H
#define PHYSICS_H

#include <glm/glm.hpp>

#include <scene/scene.h>

class Physics
{
public:
    Physics();

    glm::vec3 windDirection;
    static void setup(Scene &scene);
    static void update(Scene &scene);

private:
    glm::mat4 baseTransform;
    float steeringAngle;
    float sailAngle;
    void move();
};

#endif