#pragma once

#include "Cube.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"

class Skybox : public Demo
{
public:
	Skybox();

	void init() override;
	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Renderer renderer;
	Camera camera;

	uint32_t skyboxProgramId;
	uint32_t skyboxTextureId;
	uint32_t vertexArray;

	std::vector<uint32_t> textureIds;
};