#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "AttributeLayout.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class Renderer
{
private:
	// GLFW
	GLFWwindow* window;

	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;
	std::unique_ptr<Program> program;

	uint32_t vao;

	std::vector<Texture> textures;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 mvp;

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;

	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point lastSecondTime;
	uint32_t milliseconds;
	uint32_t leftOverMillis = 0;
	uint32_t fps = 0;
	uint32_t previousMillis = 0;

	float lastX = 0.0f;
	float lastY = 0.0f;
	float yaw = 90.0f;
	float pitch = 0.0f;

	GLFWwindow* initWindow();
	void initOpenGL();

	void calculateCameraOrientation();
	void calculateCameraPosition();

	static void mouseCallbackGLFW(GLFWwindow* window, double xPos, double yPos);
	void mouseCallback(GLFWwindow* window, double xPos, double yPos);

	uint32_t getGLTypeSize(GLenum type);

	template <class ClockType>
	uint64_t getMillisecondsSinceTimePoint(std::chrono::time_point<ClockType> start);

	static void APIENTRY debugOutputGLFW(
		GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam
	);

public:
	Renderer();

	void init();


	void generateCube(std::vector<float>& cubeVertices);

	void generateShaders();
	void generateVertexBuffer(uint32_t& vertexBuffer, const std::vector<float>& cubeVertices);
	void generateIndexBuffer(uint32_t& indexBuffer, const std::vector<float>& indices);
	void generateTexture(uint32_t& texture, const std::string& imagePath, GLenum textureUnit);
	void generateVertexArray(uint32_t& vao, uint32_t vertexBuffer, std::vector<AttributeLayout>& attribs);

	void prepareForRun();
	void calculateCameraTransform();
	void prepareForDraw();
	void updateModelMatrix(const glm::mat4& model);
	void draw(unsigned int triangleCount);
	void unprepareForDraw();
	void calculateFps();
	void updateGLFW();
	void terminateGLFW();

	void setUniform1i(const std::string& name, int32_t value);
	void setUniformMatrix4fv(const std::string& name, const glm::mat4& value);

	void testGLM();

	bool getWindowShouldClose();
	uint64_t getMillisecondsSinceRunPreparation();
	float getYaw();
	float getPitch();
};

template <class ClockType>
uint64_t Renderer::getMillisecondsSinceTimePoint(std::chrono::time_point<ClockType> start)
{
	std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
	std::chrono::duration duration = current - start;
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}