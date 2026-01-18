#include "Camera.hpp"

void Camera::updateOrientation(float yaw, float pitch)
{
    this->yaw = yaw;
    this->pitch = pitch;

    cameraDirection.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    cameraDirection.y = std::sin(glm::radians(pitch));
    cameraDirection.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

    front = glm::normalize(cameraDirection);
    right = glm::normalize(glm::cross(front, up));
}

glm::mat4 Camera::getView() const
{
    return glm::lookAt(pos, pos + front, up);
}