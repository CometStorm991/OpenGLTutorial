#include "Camera.hpp"

Camera::Camera()
    :
    pos(glm::vec3(0.0f, 0.0f, -30.0f)),
    front(glm::vec3(0.0f, 0.0f, 1.0f)),
    up(glm::vec3(0.0f, 1.0f, 0.0f)),
    right(glm::normalize(glm::cross(front, up)))
{

}

void Camera::updateOrientation(float yaw, float pitch)
{
    this->yaw = yaw;
    this->pitch = pitch;

    glm::vec3 cameraDirection;
    cameraDirection.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    cameraDirection.y = std::sin(glm::radians(pitch));
    cameraDirection.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

    front = glm::normalize(cameraDirection);
    right = glm::normalize(glm::cross(front, up));
}

float Camera::getYaw()
{
    return yaw;
}

float Camera::getPitch()
{
    return pitch;
}