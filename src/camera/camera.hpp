#pragma once

#include <glm/glm.hpp>

namespace Camera
{
    // Global Camera Variables
    inline glm::vec3 worldUp(0.f, 0.f, 1.f);
    inline glm::vec3 cameraViewDirection, cameraRight, cameraUp;

    // Position and orientation of free cam
    inline glm::vec3 cameraPosition;
    inline float yaw, pitch, roll;

    // Position and orientation of fixed cam
    inline glm::vec3 cameraPositionFree;
    inline float yawFree, pitchFree, rollFree;

    // Position and orientation of user around fixed cam
    inline float yawOffset, pitchOffset, rollOffset;

    // Camera related transformation matrices
    inline glm::mat4 u_view, u_projection, u_camXY;

    // Booleans for tracking cam state
    inline bool cameraMoved, freeCam;

    void update();
    void reset();
    void setCamDirection(glm::vec3 rotation);
    void genViewMatrix(glm::vec3 position);
    void genProjectionMatrix();

    glm::vec3 getPosition();
    glm::vec3 getRotation();
};