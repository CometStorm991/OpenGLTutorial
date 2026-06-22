#pragma once

#include <random>
#include <vector>

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "Icosahedron.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class Deferred : public Demo
{
public:
	Deferred();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	std::random_device randomDevice{};
	std::default_random_engine randomEngine{randomDevice()};

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

	uint32_t cubeVaoId = 0;
	uint32_t cubeInstBufferId = 0;
	std::vector<uint32_t> cubeTextureIds{};
	uint32_t cubeProgramId = 0;
	const uint32_t cubeCount = 100;
	std::vector<glm::vec3> cubePositions{}, cubeRotationSpeeds{};

	uint32_t posTexId = 0;
	uint32_t normTexId = 0;
	uint32_t diffSpecTexId = 0;
	uint32_t posTexUnit = 0;
	uint32_t normTexUnit = 1;
	uint32_t diffSpecTexUnit = 2;

	uint32_t lightVaoId = 0;
	const uint32_t lightCount = 50;
	std::vector<Light> lights{};
	std::vector<float> lightModelData{};
	uint32_t lightProgramId = 0;
	uint32_t lightSSBId = 0;
	uint32_t gpuLightSize = 5 * sizeof(glm::vec4);
	uint32_t lightModelSubs = 4;

	uint32_t volumeVaoId = 0;
	uint32_t volumeProgramId = 0;
	std::vector<uint32_t> volumeTexIds{};
	uint32_t volumeModelSubs = 1;
	std::vector<float> volumeModelData{};

	uint32_t geoFbId = 0;

	uint32_t debugQuadProgId = 0;
	uint32_t debugQuadVaoId = 0;
	std::vector<uint32_t> debugQuadTexIds{};

	void prepareCube();
	void updateSceneBoxData(uint64_t milliseconds);
	void prepareLight();
	void prepareVolume();
	void prepareDebugQuad();

	float getLightVolumeRadius(const Light& light);
};