#pragma once
#pragma once

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "InputState.hpp"

class Window
{
public:
	Window();
	~Window();

	void updateGLFW();
	bool getShouldClose();
	void terminate();

	const InputState& getInputState();
	static void mouseCallbackGLFW(GLFWwindow* window, double posX, double posY);

	const uint32_t width = 1920;
	const uint32_t height = 1080;
private:
	GLFWwindow* window = nullptr;
	InputState inputState{};

	void recordKeypresses();

	void mouseCallback(double posX, double posY);
	bool updatedMouse = false;
};