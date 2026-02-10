#pragma once

#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>

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

	class Light {
	public:
		glm::vec3 color;
		glm::mat4 modelMat;

		glm::vec3 pos;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float constant;
		float linear;
		float quadratic;
	};
	class GPULight {
	public:
		glm::vec4 pos;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 attentuation;
	};

	const uint32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const uint32_t SCREEN_WIDTH = 1920, SCREEN_HEIGHT = 1080;
	const float SHADOW_ASPECT = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

	const uint32_t materialDiffuseTexUnit = 0;
	const uint32_t materialSpecularTexUnit = 1;
	const uint32_t depthCubemapTexUnit = 2;
	const uint32_t materialNormalTexUnit = 3;
	const uint32_t hdrTexUnit = 4;

	uint32_t floorProgramId = 0;
	uint32_t floorVaoId = 0;
	std::vector<uint32_t> floorTextureIds{};
	float floorCubeSize = 5.0f;
	float floorCubeThickness = 1.0f;
	glm::vec3 floorCubeOffset{ 15.0f, 12.0f, 0.0f };
	glm::vec3 corridorOffset{ 0.0f, 12.0f, 20.0f };
	float corridorFactor = 10.0f;
	uint32_t floorCount = 0;
	std::vector<Floor> floors{};
	
	uint32_t lightProgramId = 0;
	uint32_t lightVaoId = 0;
	uint32_t lightCount = 0;
	glm::mat4 lightModel{1.0f};
	std::vector<Light> lights{};
	uint32_t lightSSBId = 0;
	uint32_t lightSize = 5 * sizeof(glm::vec4);
	uint32_t lightSSBSize = 0;

	uint32_t boxesProgramId = 0;
	uint32_t boxesVaoId = 0;
	std::vector<uint32_t> boxesTextureIds{};
	uint32_t boxesCount = 3;

	uint32_t wallProgramId = 0;
	uint32_t wallVaoId = 0;
	std::vector<uint32_t> wallTextureIds{};
	uint32_t wallCount = 0;

	uint32_t depthMapFbId = 0;
	uint32_t depthMapTexId = 0;
	
	uint32_t depthMapProgramId = 0;
	uint32_t depthMapInstancedProgId = 0;
	glm::mat4 lightSpaceMat{ 1.0f };

	uint32_t quadVaoId = 0;
	uint32_t quadProgramId = 0;
	std::vector<uint32_t> quadTextureIds{};

	uint32_t depthCubemapTexId = 0;
	uint32_t depthCubemapFbId = 0;
	uint32_t depthCubemapProgId = 0;
	uint32_t depthCubemapMaxCount = 9;
	float depthCubemapNear = 1.0f;
	float depthCubemapFar = 50.0f;
	uint32_t depthCubemapInstBufferId = 0;
	uint32_t depthCubemapInstSize = 0;

	uint32_t hdrFbId = 0;
	
	void buildFloorCube();
	void buildCorridor();
	void prepareFloor(const std::vector<Floor>& floors);
	void prepareBoxes();
	void prepareWalls();
	void testAddToData();
	uint32_t addToData(std::vector<float>& vertices, const std::vector<float>& data, uint32_t oldStride, uint32_t componentCount);
	void prepareLight();
	void updateLightData();
	void prepareCubemapShadows();

	void renderShadowCubemap();
	void renderScene();
};