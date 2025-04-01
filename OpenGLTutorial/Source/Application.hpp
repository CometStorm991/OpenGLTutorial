#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <STB/stb_image.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
	void generateIndexBuffer(uint32_t& indexBuffer, const std::vector<float>& indices);
	void generateTexture(uint32_t& texture, const std::string& imagePath, GLenum textureUnit);

	void testGLM();

	float getYaw();
	float getPitch();
};