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
	uint32_t vaoId;
	std::vector<uint32_t> textureIds;

	// Getting started
	const unsigned int cubeCount = 1000;
	// Desktop can handle 1200 cubes at 144 fps
	// Laptop can handle 1000 cubes at about 144 fps
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;

	// Lighting
	uint32_t lightProgramId;
	glm::vec3 lightPos;

	void generateCubePNT(std::vector<float>& vertices);
	void addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao);
	void prepareGettingStarted();
	void prepareLighting();
	
	void prepareForRun();

	void runGettingStarted();
	void runLighting();
public:
	void init();
	void prepare();
	void run();
	void terminate();

	bool shouldEnd();
};