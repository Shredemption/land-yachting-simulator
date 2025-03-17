#include "camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "event_handler/event_handler.h"

// Global Camera Variables
glm::vec3 Camera::worldUp(0.f, 0.f, 1.f);             // World up direction
glm::vec3 Camera::cameraPositionFree(17.32f, -10.f, 10.f); // Camera placed
glm::vec3 Camera::cameraPosition(0.f, 0.f, 0.f);
float Camera::yawFree = glm::radians(-60.0f), Camera::pitchFree = glm::radians(30.0f), Camera::rollFree = 0;
float Camera::yaw = 0, Camera::pitch = 0, Camera::roll = 0;
float Camera::yawOffset = 0, Camera::pitchOffset = 0, Camera::rollOffset = 0;
glm::vec3 Camera::cameraViewDirection(0.0f, 1.0f, 0.0f);
glm::vec3 Camera::cameraRight = glm::normalize(glm::cross(worldUp, -cameraViewDirection));
glm::vec3 Camera::cameraUp = glm::normalize(glm::cross(-cameraViewDirection, cameraRight));
glm::mat4 Camera::u_view;
glm::mat4 Camera::u_projection;

bool Camera::cameraMoved = true;
bool Camera::freeCam = true;

void Camera::update()
{
    setCamDirection(getRotation());
    genProjectionMatrix();
    genViewMatrix(getPos());
}

void Camera::setCamDirection(glm::vec3 rotation)
{
    float p = rotation[0];
    float y = rotation[1];
    float r = rotation[2];

    cameraViewDirection = glm::normalize(glm::vec3(cos(-p) * sin(y),
                                                   cos(-p) * cos(y),
                                                   sin(-p)));
}

// Projection Matrix
void Camera::genProjectionMatrix()
{
    u_projection = glm::perspective((float)M_PI_2,                                                        // Field of view (90 deg)
                                    (float)EventHandler::screenWidth / (float)EventHandler::screenHeight, // Aspect Ratio (w/h)
                                    0.1f,                                                                 // Near clipping plane
                                    1000.0f                                                               // Far clipping plane
    );
}

// View matrix
void Camera::genViewMatrix(glm::vec3 position)
{
    u_view = glm::lookAt(position,                       // Camera Position
                         position + cameraViewDirection, // Target Position
                         cameraUp                        // Up vector
    );
}

glm::vec3 Camera::getPos()
{
    if (freeCam)
    {
        return cameraPositionFree;
    }
    else
    {
        return cameraPosition;
    }

    return glm::vec3(0.0f);
}

glm::vec3 Camera::getRotation()
{
    if (freeCam)
    {
        return glm::vec3(pitchFree, yawFree, rollFree);
    }
    else
    {
        return glm::vec3(pitch + pitchOffset, yaw + yawOffset, roll + rollOffset);
    }

    return glm::vec3(0.0f, 0.0f, 0.0f);
}