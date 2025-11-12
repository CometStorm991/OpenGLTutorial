#pragma once

#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AttributeLayout.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "GettingStarted.hpp"
#include "MultipleLighting.hpp"
#include "Program.hpp"
#include "SimpleLighting.hpp"
#include "Shader.hpp"
#include "Rearview.hpp"
#include "Renderer.hpp"

class Application
{
private:
	std::unique_ptr<Demo> demo;
	
public:
	Application();

	void init();
	void prepare();
	void run();
	bool shouldEnd();
	void terminate();
};