#pragma once

#include "Camera.hpp"
#include "InputState.hpp"

class CameraController
{
public:
	CameraController();

	void updateCamera(const InputState& inputState, uint32_t millisecondDiff);
	void updateCameraMicroseconds(const InputState& inputState, uint32_t microsecondDiff);

	const Camera& getCamera();
	void setCameraPos(const glm::vec3& pos);
	void setCameraOrientation(float yaw, float pitch);
private:
	Camera camera;

	float sensitivity = 0.1f;

	void updateCamera(const InputState& inputState, uint32_t microsecondDiff, float xOffset, float yOffset);
};