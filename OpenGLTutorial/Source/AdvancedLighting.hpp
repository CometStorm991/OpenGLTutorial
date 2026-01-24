#pragma once

#include <cmath>

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class AdvancedLighting : public Demo
{
public:
	AdvancedLighting();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;

	void prepareFloor();
	void prepareLight();
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t floorVaoId;
	uint32_t lightVaoId;

	uint32_t programId;
	glm::vec3 pointLightPos{0.0f, 5.0f, 0.0f};
	std::vector<uint32_t> textureIds;
	glm::mat4 floorModel{1.0f};
	
	uint32_t lightProgramId;
	glm::mat4 lightModel{1.0f};
};