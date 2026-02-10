#include "CameraController.hpp"

CameraController::CameraController()
{
	
}

void CameraController::updateCamera(const InputState& inputState, uint32_t millisecondDiff)
{
    float xOffset = inputState.posX - inputState.lastX;
    float yOffset = inputState.lastY - inputState.posY; // Reversed because y coordinates range bottom up

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    float yaw = camera.yaw;
    float pitch = camera.pitch;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.updateOrientation(yaw, pitch);

    float deltaTime = millisecondDiff / 1000.0f;
    float cameraSpeed = deltaTime * 10.0f;
    if (inputState.w)
    {
        camera.pos += cameraSpeed * camera.front;
    }
    if (inputState.s)
    {
        camera.pos -= cameraSpeed * camera.front;
    }
    if (inputState.d)
    {
        camera.pos += cameraSpeed * camera.right;
    }
    if (inputState.a)
    {
        camera.pos -= cameraSpeed * camera.right;
    }
    if (inputState.e)
    {
        camera.pos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (inputState.q)
    {
        camera.pos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    if (inputState.z)
    {
        
        camera.exposure -= cameraSpeed;
        if (camera.exposure < 0.0f)
        {
            camera.exposure = 0.0f;
        }
    }
    if (inputState.x)
    {
        camera.exposure += cameraSpeed;
        if (camera.exposure > 100.0f)
        {
            camera.exposure = 100.0f;
        }
    }
}

void CameraController::setCameraPos(const glm::vec3& pos)
{
    camera.pos = pos;
}

void CameraController::setCameraOrientation(float yaw, float pitch)
{
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.updateOrientation(yaw, pitch);
}

const Camera& CameraController::getCamera()
{
    return camera;
}