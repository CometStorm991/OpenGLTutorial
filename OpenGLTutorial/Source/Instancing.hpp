#pragma once

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class Instancing : public Demo
{
public:
	Instancing();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;

	void prepareMetalCube();
	void prepareSkybox();
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t metalBoxVertexArrayId;
	uint32_t metalBoxProgramId;

	uint32_t skyboxTextureId;
	uint32_t skyboxVertexArrayId;
	uint32_t skyboxProgramId;
};