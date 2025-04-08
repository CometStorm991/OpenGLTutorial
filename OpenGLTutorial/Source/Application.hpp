#pragma once

#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AttributeLayout.hpp"
#include "Cube.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"

class Application
{
private:
	Renderer renderer;

	uint32_t programId;
	uint32_t lightProgramId;
	uint32_t vaoId;
	std::vector<uint32_t> textureIds;

	const unsigned int cubeCountGS = 1000;
	const unsigned int cubeCountML = 100;
	// Desktop can handle 1200 cubes at 144 fps
	// Laptop can handle 1000 cubes at about 144 fps
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;

	glm::vec3 simpleLightPos;
	glm::vec3 directionalLightDir;

	void addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao);
	void prepareGettingStarted();

	void addLightingInfo();
	void prepareSimpleLighting();
	void prepareMultipleLighting();
	
	void prepareForRun();

	void runGettingStarted();
	void runSimpleLighting();
	void runMultipleLighting();
public:
	void init();
	void prepare();
	void run();
	void terminate();

	bool shouldEnd();
};