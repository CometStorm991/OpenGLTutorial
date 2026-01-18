#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera
{
public:
	float yaw = 90.0f;
	float pitch = 0.0f;

	glm::vec3 pos{ 0.0f, 0.0f, -30.0f };
	glm::vec3 cameraDirection{ 0.0f, 0.0f, 0.0f };

	glm::vec3 front{ 0.0f, 0.0f, 1.0f };
	const glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 right = glm::normalize(glm::cross(front, up));

	void updateOrientation(float yaw, float pitch);
	glm::mat4 getView() const;
};