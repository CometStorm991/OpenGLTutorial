#pragma once

#include "CameraController.hpp"
#include "Cube.hpp"
#include "Demo.hpp"
#include "Icosahedron.hpp"
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

	uint32_t slices = 50, stacks = 50;
	uint32_t sphereCount = 8 * 8;
	uint32_t sphereVaoId, sphereProgramId;
	std::vector<uint32_t> sphereTexIds;

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
	uint32_t gpuLightSize = 5 * sizeof(glm::vec4);
	uint32_t lightVaoId, lightProgramId;
	std::vector<Light> lights{};
	std::vector<float> lightModelData;
	uint32_t lightCount = 64;
	uint32_t lightSSBId;
	uint32_t lightModelSubs = 0;

	uint32_t hdrFbId;
	uint32_t hdrTexUnit = 0;
	uint32_t hdrProgramId, hdrVaoId;
	std::vector<uint32_t> hdrTexIds;

	uint32_t envTexId, envTexUnit = 0;
	uint32_t envCubemapTexId, envCubemapTexUnit = 0;
	uint32_t envCubemapLength = 2560;
	uint32_t envCubemapMipLevels = 1 + (uint32_t)std::floor(std::log2(envCubemapLength));
	uint32_t envCubemapFbId;
	uint32_t envCubemapProgramId, envCubemapVaoId;
	std::vector<uint32_t> envCubemapTexIds{};

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	std::array<glm::mat4, 6> captureViews = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	uint32_t irradTexId, irradTexUnit = 4;
	uint32_t irradCubemapLength = 32;
	uint32_t irradProgramId;
	std::vector<uint32_t> irradTexIds{};

	uint32_t prefTexId, prefTexUnit = 5;
	uint32_t prefMipLevels = 5;
	uint32_t prefCubemapLength = 128;
	uint32_t prefProgramId;
	std::vector<uint32_t> prefTexIds{};

	uint32_t skyboxVaoId, skyboxProgramId;
	std::vector<uint32_t> skyboxTexIds{};

	void prepareEnvMap();
	void prepareEnvCubemap();
	void prepareIrradiance();
	void preparePrefiltered();
	void prepareSkybox();
	void prepareSpheres();
	void prepareLights();
	void prepareHDR();
};