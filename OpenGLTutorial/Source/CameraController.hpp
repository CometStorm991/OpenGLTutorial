#pragma once

#include "Camera.hpp"
#include "InputState.hpp"

class CameraController
{
public:
	CameraController();

	void updateCamera(const InputState& inputState, uint32_t millisecondDiff);

	const Camera& getCamera();
	void setCameraPos(const glm::vec3& pos);
	void setCameraOrientation(float yaw, float pitch);
private:
	Camera camera;

	float sensitivity = 0.1f;
};