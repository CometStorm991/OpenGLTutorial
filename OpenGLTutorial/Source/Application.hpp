#pragma once

#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AttributeLayout.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"

class Application
{
private:
	Renderer renderer;

	const unsigned int cubeCount = 1000;
	// Desktop can handle 1200 cubes at 144 fps
	// Laptop can handle 1000 cubes at about 144 fps
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;

	void addCubeVertices();
	void prepareRotatingCubes();
	void prepareLighting();
	
	void prepareForRun();
public:
	void init();
	void prepare();
	void run();
	void terminate();

	bool shouldEnd();
};