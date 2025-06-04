#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>

#include <string>

class Physics
{
public:
    // Constructor
    Physics(const std::string &name);
    
    void move(bool &controlled);
    void reset(const glm::mat4 &u_model);
    void savePrevState();
    void copyFrom(const Physics &other);

    // Velocity and steering variables
    glm::mat4 baseTransform, prevBaseTransform;
    float steeringAngle = 0.0f, prevSteeringAngle;
    float steeringSmoothness;
    float maxSteeringAngle;
    float steeringAttenuation;
    float forwardVelocity = 0.0f;
    float wheelAngle = 0.0f, prevWheelAngle;

    // Mast/sail/boom angles
    float maxMastAngle;
    float maxBoomAngle;
    float MastAngle = 0.0f, prevMastAngle;
    float BoomAngle = 0.0f, prevBoomAngle;
    float sailControlFactor = 1.0f;
    float optimalAngle;
    float SailAngle = 0.0f, prevSailAngle;

    // Friction/drag coefficients
    float maxLiftCoefficient;
    float minDragCoefficient;
    float rollCoefficient;
    float rollScaling;

    // Body size properties
    float sailArea;
    float mass;
    float bodyDragCoefficient;
    float bodyArea;
};

#endif