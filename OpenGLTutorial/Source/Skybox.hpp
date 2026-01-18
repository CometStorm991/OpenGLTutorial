#pragma once

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class Skybox : public Demo
{
public:
	Skybox();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;

	void prepareBox();
	void prepareReflectiveBox();
	void prepareSkybox();
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t boxProgramId = 0;
	uint32_t boxVertexArrayId = 0;

	uint32_t reflectiveBoxProgramId = 0;
	uint32_t reflectiveBoxVertexArrayId = 0;

	uint32_t skyboxProgramId = 0;
	uint32_t skyboxTextureId = 0;
	uint32_t skyboxVertexArrayId = 0;


	std::vector<uint32_t> boxTextureIds{};
};