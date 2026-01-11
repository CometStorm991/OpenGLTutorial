#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "AttributeLayout.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class Renderer
{
private:
	// GLFW
	GLFWwindow* window = nullptr;

	std::unordered_map<uint32_t, Program> programMap;
	std::unordered_map<uint32_t, Texture> textureMap;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 mvp;

	Camera& camera;

	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point lastSecondTime;
	uint32_t milliseconds = 0;
	uint32_t leftOverMillis = 0;
	uint32_t fps = 0;
	uint32_t previousMillis = 0;

	float lastX = 0.0f;
	float lastY = 0.0f;

	GLFWwindow* initWindow();
	void initOpenGL();

	void updateCameraPosition();

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
	Renderer(Camera& camera);

	void init();

	void generateProgram(uint32_t& programId, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void generateVertexBuffer(uint32_t& vertexBuffer, const std::vector<float>& cubeVertices);
	void generateIndexBuffer(uint32_t& indexBuffer, const std::vector<uint32_t>& indices);
	void generateTexture(uint32_t& textureId, const std::string& imagePath, GLenum pixelFormat);
	void generateTexture(uint32_t& textureId, uint32_t width, uint32_t height);
	void generateVertexArray(uint32_t& vao, uint32_t vertexBuffer, uint32_t indexBuffer, std::vector<AttributeLayout>& attribs);
	void generateFramebuffer(uint32_t& framebufferId);

	void prepareForRun();
	void prepareForRender();
	void prepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds, uint32_t vaoId);
	void updateModelMatrix(const glm::mat4& model);
	void applyMvp(uint32_t programId, const std::string& modelName, const std::string& viewName, const std::string& projectionName);
	void draw(unsigned int triangleCount);
	void unprepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds);
	void calculateFps();
	void updateGLFW();
	void terminateGLFW();

	void setUniform1i(uint32_t programId, const std::string& name, int32_t value);
	void setUniform1f(uint32_t programId, const std::string& name, float value);
	void setUniform3f(uint32_t programId, const std::string& name, const glm::vec3& value);
	void setUniformMatrix4fv(uint32_t programId, const std::string& name, const glm::mat4& value);

	void testGLM();

	bool getWindowShouldClose();
	uint64_t getMillisecondsSinceRunPreparation();
};

template <class ClockType>
uint64_t Renderer::getMillisecondsSinceTimePoint(std::chrono::time_point<ClockType> start)
{
	std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
	std::chrono::duration duration = current - start;
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}