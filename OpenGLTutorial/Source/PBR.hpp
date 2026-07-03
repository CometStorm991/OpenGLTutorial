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
};