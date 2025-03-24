#include "camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "event_handler/event_handler.h"
#include "scene_manager/scene_manager.h"

// Global Camera Variables
glm::vec3 Camera::worldUp(0.f, 0.f, 1.f); // World up direction
glm::vec3 Camera::cameraPositionFree;
glm::vec3 Camera::cameraPosition;
float Camera::yawFree, Camera::pitchFree, Camera::rollFree;
float Camera::yaw, Camera::pitch, Camera::roll;
float Camera::yawOffset, Camera::pitchOffset, Camera::rollOffset;
glm::vec3 Camera::cameraViewDirection;
glm::vec3 Camera::cameraRight;
glm::vec3 Camera::cameraUp;
glm::mat4 Camera::u_view;
glm::mat4 Camera::u_projection;
glm::mat4 Camera::u_camXY;

bool Camera::cameraMoved;
bool Camera::freeCam;

void Camera::update()
{
    setCamDirection(getRotation());
    genProjectionMatrix();
    glm::vec3 position = getPosition();
    genViewMatrix(position);
    u_camXY = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], 0));
}

void Camera::reset()
{
    if (SceneManager::onTitleScreen)
    {
        cameraPosition = glm::vec3(0.f, 0.f, 0.f);
        yaw = 0, pitch = 0, roll = 0;
        yawOffset = 0, pitchOffset = 0, rollOffset = 0;

        cameraMoved = true;
        freeCam = false;

        setCamDirection(getRotation());
        genViewMatrix(getPosition());
    }
    else
    {
        cameraPositionFree = glm::vec3(-0.5f, -12.f, 3.f);
        cameraPosition = glm::vec3(0.f, 0.f, 0.f);

        yawFree = glm::radians(-40.0f), pitchFree = 0, rollFree = 0;
        yaw = 0, pitch = 0, roll = 0;
        yawOffset = 0, pitchOffset = 0, rollOffset = 0;

        cameraMoved = true;
        freeCam = true;

        setCamDirection(getRotation());
        genViewMatrix(getPosition());
    }
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
    u_projection = glm::perspective(glm::radians(75.0f),
                                    (float)EventHandler::screenWidth / (float)EventHandler::screenHeight,
                                    0.1f,
                                    1000.0f);
}

// View matrix
void Camera::genViewMatrix(glm::vec3 position)
{
    cameraRight = glm::normalize(glm::cross(worldUp, -cameraViewDirection));
    cameraUp = glm::normalize(glm::cross(-cameraViewDirection, cameraRight));

    u_view = glm::lookAt(position,                       // Camera Position
                         position + cameraViewDirection, // Target Position
                         cameraUp                        // Up vector
    );
}

glm::vec3 Camera::getPosition()
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