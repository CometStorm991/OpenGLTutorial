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
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	class Floor {
	public:
		float texScale;
		glm::mat4 modelMat;
	};

	const uint32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const uint32_t SCREEN_WIDTH = 1920, SCREEN_HEIGHT = 1080;

	glm::vec3 ptLightPos{ 0.0f, 5.0f, 0.0f };
	glm::vec3 ptLightAmbient{ 0.2f, 0.2f, 0.2f };
	glm::vec3 ptLightDiffuse{ 0.5f, 0.5f, 0.5f };
	glm::vec3 ptLightSpecular{ 1.0f, 1.0f, 1.0f };
	float ptLightConstant = 1.0f;
	float ptLightLinear = 0.07f;
	float ptLightQuadratic = 0.017f;

	uint32_t floorVaoId = 0;
	uint32_t boxesVaoId = 0;
	uint32_t lightVaoId = 0;

	const uint32_t materialDiffuseTexUnit = 0;
	const uint32_t materialSpecularTexUnit = 1;
	const uint32_t depthMapTexUnit = 2;
	const uint32_t depthCubemapTexUnit = 3;

	uint32_t floorProgramId = 0;
	std::vector<uint32_t> floorTextureIds{};

	float floorCubeSize = 5.0f;
	float floorCubeThickness = 1.0f;
	glm::vec3 floorCubeOffset{ 15.0f, 12.0f, 0.0f };
	uint32_t floorCount = 0;
	
	uint32_t lightProgramId = 0;
	glm::mat4 lightModel{1.0f};

	uint32_t boxesProgramId = 0;
	std::vector<uint32_t> boxesTextureIds{};
	uint32_t boxesCount = 3;

	uint32_t depthMapFbId = 0;
	uint32_t depthMapTexId = 0;
	
	uint32_t depthMapProgramId = 0;
	uint32_t depthMapInstancedProgId = 0;
	glm::mat4 lightSpaceMat{ 1.0f };

	uint32_t quadVaoId = 0;
	uint32_t quadProgramId = 0;

	uint32_t depthCubemapTexId = 0;
	uint32_t depthCubemapFbId = 0;
	uint32_t depthCubemapProgId = 0;

	
	void buildFloorCube(std::vector<Floor>& floors);
	void prepareFloor(const std::vector<Floor>& floors);
	void prepareBoxes();
	void prepareLight();
	void prepareShadows();
	void prepareCubemapShadows();

	void renderShadowMap();
	void renderShadowCubemap();
	void renderScene();

	void applyLightUniforms(uint32_t floorProgramId);
};