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
#include "Instancing.hpp"
#include "MultipleLighting.hpp"
#include "Program.hpp"
#include "Rearview.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "SimpleLighting.hpp"
#include "Skybox.hpp"

class Application
{
private:
	std::unique_ptr<Demo> demo;
	
public:
	Application();

	void prepare();
	void run();
	bool shouldEnd();
	void terminate();
};