#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cube.hpp"
#include "Camera.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"

class Rearview : public Demo
{
public:
	Rearview();

	void init() override;
	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Renderer renderer;
	Camera camera;

	uint32_t vaoId;
	uint32_t quadVaoId;
	uint32_t programId;
	uint32_t lightProgramId;
	uint32_t quadProgramId;
	std::vector<uint32_t> textureIds;

	const uint32_t cubeCount = 100;
	std::vector<glm::vec3> cubePositions;
	std::vector<glm::vec3> cubeRotationSpeeds;
	glm::vec3 pointLightPos;
	glm::vec3 directionalLightDir;

	uint32_t framebuffer;
	uint32_t textureColorBuffer;

	void addLightingInfo();
};