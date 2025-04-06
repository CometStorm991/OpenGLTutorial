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
#include "Program.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class Renderer
{
private:
	// GLFW
	GLFWwindow* window;

	/*std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;
	std::unique_ptr<Program> program;*/

	std::unordered_map<uint32_t, Program> programMap;
	std::unordered_map<uint32_t, Texture> textureMap;

	/*std::vector<Texture> textures;*/

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

	void generateProgram(uint32_t& programId, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void generateVertexBuffer(uint32_t& vertexBuffer, const std::vector<float>& cubeVertices);
	void generateIndexBuffer(uint32_t& indexBuffer, const std::vector<float>& indices);
	void generateTexture(uint32_t& textureId, const std::string& imagePath, GLenum textureUnit);
	void generateVertexArray(uint32_t& vao, uint32_t vertexBuffer, std::vector<AttributeLayout>& attribs);

	void prepareForRun();
	void setCameraPos(const glm::vec3& cameraPos);
	void calculateCameraTransform();
	void prepareForRender();
	void prepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds, uint32_t vaoId);
	void updateModelMatrix(const glm::mat4& model);
	void applyMvp(uint32_t programId, const std::string& modelName, const std::string& viewName, const std::string& projectionName);
	void draw(unsigned int triangleCount);
	void unprepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds);
	void calculateFps();
	void updateGLFW();
	void terminateGLFW();

	glm::vec3 getCameraPos();

	void setUniform1i(uint32_t programId, const std::string& name, int32_t value);
	void setUniform3f(uint32_t programId, const std::string& name, const glm::vec3& value);
	void setUniformMatrix4fv(uint32_t programId, const std::string& name, const glm::mat4& value);

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