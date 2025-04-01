#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Application
{
private:
	// GLFW
	GLFWwindow* window;

	float lastX = 0.0f;
	float lastY = 0.0f;
	float yaw = 90.0f;
	float pitch = 0.0f;

	static void APIENTRY debugOutputGLFW(
		GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam
	);
	static void mouseCallbackGLFW(GLFWwindow* window, double xPos, double yPos);

	void mouseCallback(GLFWwindow* window, double xPos, double yPos);
public:
	Application();
	
	GLFWwindow* initWindow();
	void initOpenGL();

	void generateCube(std::vector<float>& cubeVertices);

	void generateVertexBuffer(uint32_t& vertexBuffer, const std::vector<float>& cubeVertices);

	float getYaw();
	float getPitch();
};