#include "SSAO.hpp"

SSAO::SSAO()
{

}

void SSAO::prepare()
{
	renderer.generateProgram(ayaProgramId, "Shaders/SSAO/ModelVS.glsl", "Shaders/SSAO/ModelFS.glsl");
	glm::mat4 ayaModelMat = glm::mat4{ 1.0f };
	ayaModelMat = glm::translate(ayaModelMat, glm::vec3{ 0.0f, -10.0f, -15.0f });
	ayaModelMat = glm::rotate(ayaModelMat, static_cast<float>(std::numbers::pi), glm::vec3{ 0.0f, 1.0f, 0.0f });
	ayaModelMat = glm::scale(ayaModelMat, glm::vec3{ 0.01f, 0.01f, 0.01f });
	renderer.setUniformMatrix4fv(ayaProgramId, "model", ayaModelMat);

	prepareDeferred();
	prepareLights();
	prepareVolume();
	

	renderer.prepareForRun();
}

void SSAO::prepareDeferred()
{
	// Position
	glCreateTextures(GL_TEXTURE_2D, 1, &posTexId);
	glTextureStorage2D(posTexId, 1, GL_RGBA16F, window.width, window.height);

	// Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &normTexId);
	glTextureStorage2D(normTexId, 1, GL_RGBA16F, window.width, window.height);

	// Diffuse + specular
	glCreateTextures(GL_TEXTURE_2D, 1, &diffSpecTexId);
	glTextureStorage2D(diffSpecTexId, 1, GL_RGBA16F, window.width, window.height);

	uint32_t rendBufId;
	renderer.generateRenderbuffer(rendBufId, window.width, window.height);

	renderer.generateFramebuffer(geoFbId, {
		{ GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexId },
		{ GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normTexId },
		{ GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, diffSpecTexId },
		{ GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendBufId }, });
	//glNamedFramebufferRenderbuffer(0, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendBufId);

	uint32_t attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glNamedFramebufferDrawBuffers(geoFbId, 3, attachments);

	if (glCheckNamedFramebufferStatus(geoFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "[Error] Framebuffer is not complete!" << std::endl;
	}

	renderer.addTexture(posTexId, GL_TEXTURE_2D, posTexUnit);
	renderer.addTexture(normTexId, GL_TEXTURE_2D, normTexUnit);
	renderer.addTexture(diffSpecTexId, GL_TEXTURE_2D, diffSpecTexUnit);
}

void SSAO::prepareVolume()
{
	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = renderer.addToData(vertices, Icosahedron::fillP(volumeModelSubs), stride, 3);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(volumeVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(volumeProgramId, "Shaders/SSAO/VolumeVS.glsl", "Shaders/SSAO/VolumeFS.glsl");
	renderer.generateProgram(stencilProgramId, {
		{GL_VERTEX_SHADER, "Shaders/SSAO/StencilVS.glsl"},
		{GL_FRAGMENT_SHADER, "Shaders/SSAO/StencilFS.glsl"} });

	renderer.setUniform1i(volumeProgramId, "posSamp", posTexUnit);
	renderer.setUniform1i(volumeProgramId, "normSamp", normTexUnit);
	renderer.setUniform1i(volumeProgramId, "albedoSpecSamp", diffSpecTexUnit);
	volumeTexIds = { posTexId, normTexId, diffSpecTexId };

	renderer.setUniform2f(volumeProgramId, "screenDims", glm::vec2{ window.width, window.height });

	volumeModelData = std::vector<float>{};
	volumeModelData.reserve(lightCount * 16);
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		glm::mat4 model = glm::mat4{ 1.0f };
		float radius = getLightVolumeRadius(lights[i]);
		model = glm::translate(model, lights[i].pos);
		model = glm::scale(model, glm::vec3{ radius });
		const float* modelPtr = glm::value_ptr(model);
		volumeModelData.insert(volumeModelData.end(), modelPtr, modelPtr + 16);
	}

	std::vector<float> instData;
	uint32_t instStride = 0;

	instStride = renderer.addToData(instData, volumeModelData, instStride, 16);

	uint32_t instBuffer;
	renderer.generateVertexBuffer(instBuffer, instData);

	std::vector<AttributeLayout> instAttribs = {
		{4, GL_FLOAT, 1},
		{4, GL_FLOAT, 2},
		{4, GL_FLOAT, 3},
		{4, GL_FLOAT, 4}
	};

	renderer.addInstToVertexArray(volumeVaoId, instBuffer, instAttribs);

	renderer.setUniform1f(volumeProgramId, "materialShininess", 4.0f);
}

float SSAO::getLightVolumeRadius(const Light& light)
{
	const float kc = light.constant;
	const float kl = light.linear;
	const float kq = light.quadratic;
	const float iMax = std::fmaxf(std::fmaxf(light.color.r, light.color.g), light.color.b);
	const float sqrtDiscrim = std::sqrtf(kl * kl - 4.0 * kq * (kc - iMax * 256.0f / 5.0f));
	return (-kl + sqrtDiscrim) / (2.0f * kq);
}

void SSAO::prepareLights()
{
	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = renderer.addToData(vertices, Icosahedron::fillP(lightModelSubs), stride, 3);
	stride = renderer.addToData(vertices, Icosahedron::fillN(lightModelSubs), stride, 3);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };
	AttributeLayout normAttrib{ 3, GL_FLOAT, 1 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);

	renderer.generateVertexArray(lightVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(lightProgramId, "Shaders/SSAO/LightBoxVS.glsl", "Shaders/SSAO/LightBoxFS.glsl");

	for (unsigned int i = 0; i < lightCount; i++)
	{
		glm::vec3 color{
			1.0f, 1.0f, 1.0f
		};

		float posX = (i % 4 / 1) * 10.0f - 15.0f;
		float posY = (i % 16 / 4) * 10.0f - 17.0f;
		float posZ = (i % 64 / 16) * 10.0f - 31.0f;

		glm::vec3 pos{
			posX, posY, posZ
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

	lightModelData = std::vector<float>{};
	lightModelData.reserve(lightCount * 16);
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		const float* modelPtr = glm::value_ptr(lights[i].modelMat);
		lightModelData.insert(lightModelData.end(), modelPtr, modelPtr + 16);
	}

	std::vector<float> instData;
	uint32_t instStride = 0;

	instStride = renderer.addToData(instData, lightModelData, instStride, 16);

	uint32_t instBuffer;
	renderer.generateVertexBuffer(instBuffer, instData);

	std::vector<AttributeLayout> instAttribs = {
		{4, GL_FLOAT, 2},
		{4, GL_FLOAT, 3},
		{4, GL_FLOAT, 4},
		{4, GL_FLOAT, 5}
	};

	renderer.addInstToVertexArray(lightVaoId, instBuffer, instAttribs);
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
	renderer.bindFramebuffer(geoFbId);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xFF);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// Gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);

	Camera camera = camController.getCamera();
	glm::mat4 view = camera.getView();
	glm::vec3 pos = camController.getCamera().pos;

	
	{
		model.draw(geoFbId, ayaProgramId, view, pos);
	}

	{
		renderer.prepareForDraw(geoFbId, stencilProgramId, {}, volumeVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0x00);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilMask(0xFF);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(stencilProgramId, "", "view", "projection");
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, volumeModelSubs)), lightCount);

		renderer.unprepareForDraw(stencilProgramId, {});
	}

	{
		renderer.prepareForDraw(0, volumeProgramId, volumeTexIds, volumeVaoId);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glStencilMask(0x00);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		glBlitNamedFramebuffer(
			geoFbId, 0, 0, 0, window.width, window.height, 0, 0, window.width, window.height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
		);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(volumeProgramId, "", "view", "projection");
		renderer.setUniform3f(volumeProgramId, "viewPos", pos);
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, volumeModelSubs)), lightCount);

		renderer.unprepareForDraw(volumeProgramId, volumeTexIds);
	}

	{
		renderer.prepareForDraw(0, lightProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(lightProgramId, "", "view", "projection");
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, lightModelSubs)), lightCount);

		renderer.unprepareForDraw(lightProgramId, {});
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