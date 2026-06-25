#pragma once

#include <numbers>
#include <random>

#include "CameraController.hpp"
#include "Demo.hpp"
#include "Model.hpp"
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

	uint32_t programId;

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

	std::vector<Light> lights{};
	uint32_t lightCount = 50;
	uint32_t lightSSBId;
};