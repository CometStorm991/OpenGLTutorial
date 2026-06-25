#include "SSAO.hpp"

SSAO::SSAO()
{

}

void SSAO::prepare()
{
	renderer.generateProgram(programId, "Shaders/SSAO/ModelVS.glsl", "Shaders/SSAO/ModelFS.glsl");

	std::uniform_real_distribution<float> colorDistrib{ 0.0f, 1.0f };
	std::uniform_real_distribution<float> posDistrib{ -20.0f, 20.0f };
	for (unsigned int i = 0; i < lightCount; i++)
	{
		//glm::vec3 color{
		//	colorDistrib(randomEngine),
		//	colorDistrib(randomEngine),
		//	colorDistrib(randomEngine),
		//};
		glm::vec3 color{
			1.0f, 1.0f, 1.0f
		};

		glm::vec3 pos{
			posDistrib(randomEngine),
			posDistrib(randomEngine),
			posDistrib(randomEngine),
		};

		glm::mat4 modelMat = glm::mat4{ 1.0f };
		modelMat = glm::translate(modelMat, pos);
		modelMat = glm::scale(modelMat, glm::vec3{ 0.5f, 0.5f, 0.5f });

		glm::vec3 ambient = 0.2f * color;
		glm::vec3 diffuse = color;
		//glm::vec3 specular{ 1.0f, 1.0f, 1.0f };
		glm::vec3 specular = color;

		float constant = 1.0f;
		float linear = 0.22f;
		float quadratic = 0.20f;

		lights.emplace_back(color, modelMat, pos, ambient, diffuse, specular, constant, linear, quadratic);
	}

	uint32_t lightSSBSize = lights.size() * gpuLightSize;
	glCreateBuffers(1, &lightSSBId);
	std::vector<GPULight> gpuLights;
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		gpuLights.push_back({
			glm::vec4{ lights[i].pos, 0.0f },
			glm::vec4{ lights[i].ambient, 0.0f },
			glm::vec4{ lights[i].diffuse, 0.0f },
			glm::vec4{ lights[i].specular, 0.0f },
			glm::vec4{ lights[i].constant, lights[i].linear, lights[i].quadratic, 0.0f },
			});
	}
	glNamedBufferStorage(lightSSBId, lightSSBSize, gpuLights.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBId);

	renderer.prepareForRun();
}

void SSAO::run()
{
	renderer.prepareForFrame();

	renderer.bindFramebuffer(0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xFF);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	Camera camera = camController.getCamera();
	glm::mat4 view = camera.getView();
	glm::vec3 pos = camController.getCamera().pos;

	glm::mat4 modelMat = glm::mat4{ 1.0f };
	modelMat = glm::translate(modelMat, glm::vec3{ 0.0f, -10.0f, -15.0f });
	modelMat = glm::rotate(modelMat, static_cast<float>(std::numbers::pi), glm::vec3{ 0.0f, 1.0f, 0.0f });
	modelMat = glm::scale(modelMat, glm::vec3{ 0.01f, 0.01f, 0.01f });
	renderer.setUniformMatrix4fv(programId, "model", modelMat);
	{
		model.draw(programId, view, pos);
		chairModel.draw(programId, view, pos);
	}

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

bool SSAO::shouldEnd()
{
	return window.getShouldClose();
}

void SSAO::terminate()
{
	window.terminate();
}