#pragma once

#include "CameraController.hpp"
#include "Demo.hpp"
#include "Renderer.hpp"
#include "UVSphere.hpp"
#include "Window.hpp"

class PBR : public Demo
{
public:
	PBR();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	uint32_t slices = 10, stacks = 10;
	uint32_t sphereCount = 8 * 8;
	uint32_t sphereVaoId, sphereProgramId;
	std::vector<uint32_t> sphereTexIds;

	uint32_t hdrFbId;
	uint32_t hdrTexUnit = 0;
	uint32_t hdrProgramId, hdrVaoId;
	std::vector<uint32_t> hdrTexIds;

	void prepareSpheres();
	void prepareHDR();
};