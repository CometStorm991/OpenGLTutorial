#pragma once

#include <cstdint>
#include <vector>

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Camera.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class SimpleLighting : public Demo
{
public:
	SimpleLighting();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	void addLightingInfo();

	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t vaoId;
	uint32_t programId;
	uint32_t lightProgramId;
	std::vector<uint32_t> textureIds;

	glm::vec3 simpleLightPos;
};