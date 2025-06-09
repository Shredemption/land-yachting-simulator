#include "camera/camera.hpp"

#include "pch.h"

// Update cam matrices from positions etc
void Camera::update()
{
    setCamDirection(getRotation());
    genProjectionMatrix();
    glm::vec3 position = getPosition();
    genViewMatrix(position);
    u_camXY = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], 0));
}

// Reset cam to starting position/orientation
void Camera::reset()
{
    if (SceneManager::engineState == EngineState::Title)
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

// Set camera direction from rotation angles
void Camera::setCamDirection(glm::vec3 rotation)
{
    float p = rotation[0]; // pitch
    float y = rotation[1]; // yaw
    float r = rotation[2]; // roll

    cameraViewDirection = glm::normalize(glm::vec3(cos(-p) * sin(y),
                                                   cos(-p) * cos(y),
                                                   sin(-p)));
}

// Projection Matrix
void Camera::genProjectionMatrix()
{
    u_projection = glm::perspective(glm::radians(75.0f),
                                    (float)WindowManager::screenWidth / (float)WindowManager::screenHeight,
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

// Get camera position
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

    // Fallback value
    return glm::vec3(0.0f);
}

// Get camera orientation
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

    // Fallback value
    return glm::vec3(0.0f);
}