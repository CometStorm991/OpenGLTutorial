#pragma once

#include <stdexcept>
#include <numbers>
#include <random>

#include "CameraController.hpp"
#include "Demo.hpp"
#include "Model.hpp"
#include "Icosahedron.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

class SSAO : public Demo
{
public:
	SSAO();

	void prepare() override;
	void run() override;
	bool shouldEnd() override;
	void terminate() override;
private:
	Window window{};
	Renderer renderer{};
	CameraController camController{};

	std::random_device randomDevice{};
	std::default_random_engine randomEngine{ randomDevice() };

	uint32_t geoFbId;
	uint32_t posTexId = 0;
	uint32_t normTexId = 0;
	uint32_t diffSpecTexId = 0;
	uint32_t ssaoTexUnit = 0;
	uint32_t posTexUnit = 0;
	uint32_t normTexUnit = 1;
	uint32_t diffSpecTexUnit = 2;
	uint32_t noiseTexUnit = 2;
	uint32_t blurTexUnit = 3;

	uint32_t ayaProgramId;

	// Download from https://free3d.com/3d-model/091_aya-3dsmax-2020-189298.html
	Model model{"Resources/AyaModel/091_W_Aya_100K.obj", renderer};

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

	uint32_t volumeVaoId, volumeProgramId;
	std::vector<float> volumeModelData{};
	std::vector<uint32_t> volumeTexIds{};
	uint32_t volumeModelSubs = 1;

	uint32_t stencilProgramId = 0;

	uint32_t kernelSampleCount = 64;
	std::vector<glm::vec3> kernelSamples{};

	uint32_t noiseTexId;
	uint32_t noiseTexLen = 4;

	std::vector<uint32_t> ssaoTexIds;
	uint32_t ssaoFbId;
	uint32_t ssaoTexId;
	uint32_t ssaoVaoId, ssaoProgramId;

	uint32_t blurFbId;
	uint32_t blurTexId;
	uint32_t blurProgramId;
	std::vector<uint32_t> blurTexIds;

	void prepareDeferred();
	void prepareVolume();
	float getLightVolumeRadius(const Light& light);
	void prepareLights();
	void prepareSSAOAssets();
	void prepareSSAO();
	float lerp(float a, float b, float f);
};