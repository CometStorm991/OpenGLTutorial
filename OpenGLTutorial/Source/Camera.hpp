#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera
{
private:
	float yaw = 90.0f;
	float pitch = 0.0f;
public:
	glm::vec3 pos;
	glm::vec3 front;
	const glm::vec3 up;
	glm::vec3 right;

	Camera();
	void updateOrientation(float yaw, float pitch);

	float getYaw();
	float getPitch();
};