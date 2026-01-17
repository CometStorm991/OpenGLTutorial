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

	void prepareBox();
	void prepareReflectiveBox();
	void prepareSkybox();
private:
	Renderer renderer;
	Camera camera;

	uint32_t boxProgramId;
	uint32_t boxVertexArrayId;

	uint32_t reflectiveBoxProgramId;
	uint32_t reflectiveBoxVertexArrayId;

	uint32_t skyboxProgramId;
	uint32_t skyboxTextureId;
	uint32_t skyboxVertexArrayId;


	std::vector<uint32_t> boxTextureIds;
};