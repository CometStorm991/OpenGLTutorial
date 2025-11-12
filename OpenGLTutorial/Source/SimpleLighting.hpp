#pragma once

#include <cstdint>
#include <vector>

#include "Cube.hpp"
#include "Camera.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"

class SimpleLighting : public Demo
{
public:
	SimpleLighting();

	void init() override;
	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	void addLightingInfo();

	Renderer renderer;
	Camera camera;

	uint32_t vaoId;
	uint32_t programId;
	uint32_t lightProgramId;
	std::vector<uint32_t> textureIds;

	glm::vec3 simpleLightPos;
};