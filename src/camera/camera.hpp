#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera
{
public:
    // Global Camera Variables
    static glm::vec3 worldUp, cameraViewDirection, cameraRight, cameraUp;

    // Position and orientation of free cam
    static glm::vec3 cameraPosition;
    static float yaw, pitch, roll;

    // Position and orientation of fixed cam
    static glm::vec3 cameraPositionFree;
    static float yawFree, pitchFree, rollFree;

    // Position and orientation of user around fixed cam
    static float yawOffset, pitchOffset, rollOffset;

    // Camera related transformation matrices
    static glm::mat4 u_view, u_projection, u_camXY;

    // Booleans for tracking cam state
    static bool cameraMoved, freeCam;

    static void update();
    static void reset();
    static void setCamDirection(glm::vec3 rotation);
    static void genViewMatrix(glm::vec3 position);
    static void genProjectionMatrix();

    static glm::vec3 getPosition();
    static glm::vec3 getRotation();
};

#endif