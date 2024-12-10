#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
public:
    // Global Camera Variables
    static glm::vec3 worldUp, cameraPosition, cameraViewDirection, cameraRight, cameraUp;
    static float yaw, pitch, roll;

    static glm::mat4 u_view;
    static glm::mat4 u_projection;

    static void setCamDirection();
    static void genViewMatrix();
    static void genProjectionMatrix();
};

#endif