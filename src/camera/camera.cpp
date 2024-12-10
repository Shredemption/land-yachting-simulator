#include <camera/camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <event_handler/event_handler.h>

// Global Camera Variables
glm::vec3 Camera::worldUp(0.f, 1.f, 0.f);        // World up direction
glm::vec3 Camera::cameraPosition(0.f, 0.f, 5.f); // Camera placed
float Camera::yaw = 0, Camera::pitch = 0, Camera::roll = 0;
glm::vec3 Camera::cameraViewDirection(0.0f, 0.0f, -1.0f);
glm::vec3 Camera::cameraRight = glm::normalize(glm::cross(worldUp, -cameraViewDirection));
glm::vec3 Camera::cameraUp = glm::normalize(glm::cross(-cameraViewDirection, cameraRight));
glm::mat4 Camera::u_view;
glm::mat4 Camera::u_projection;

void Camera::setCamDirection()
{
    cameraViewDirection = glm::normalize(glm::vec3(cos(-pitch) * sin(-yaw + glm::radians(180.f)), sin(-pitch),
                                                   cos(-pitch) * cos(-yaw + glm::radians(180.f))));
}

// Projection Matrix
void Camera::genProjectionMatrix()
{
    u_projection = glm::perspective((float)M_PI_2,                                                        // Field of view (90 deg)
                                              (float)EventHandler::screenWidth / (float)EventHandler::screenHeight, // Aspect Ratio (w/h)
                                              0.01f,                                                                // Near clipping plane
                                              100.0f                                                                // Far clipping plane
    );
}

// View matrix
void Camera::genViewMatrix()
{
    u_view = glm::lookAt(cameraPosition,                       // Camera Position
                                   cameraPosition + cameraViewDirection, // Target Position
                                   cameraUp                              // Up vector
    );
}