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
	ayaModelMat = glm::scale(ayaModelMat, glm::vec3{ 0.02f, 0.02f, 0.02f });
	renderer.setUniformMatrix4fv(ayaProgramId, "model", ayaModelMat);

	prepareDeferred();
	prepareSSAO();
	prepareLights();
	prepareVolume();
	prepareHDR();

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

	// SSAO input position
	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoPosTexId);
	glTextureStorage2D(ssaoPosTexId, 1, GL_RGBA16F, window.width, window.height);
	glTextureParameteri(ssaoPosTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoPosTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoPosTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ssaoPosTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// SSAO input normal
	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoNormTexId);
	glTextureStorage2D(ssaoNormTexId, 1, GL_RGBA16F, window.width, window.height);

	uint32_t rendBufId;
	renderer.generateRenderbuffer(rendBufId, window.width, window.height);

	renderer.generateFramebuffer(geoFbId, {
		{ GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexId },
		{ GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normTexId },
		{ GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, diffSpecTexId },
		{ GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, ssaoPosTexId },
		{ GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, ssaoNormTexId },
		{ GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendBufId }, });
	//glNamedFramebufferRenderbuffer(0, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendBufId);

	uint32_t attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glNamedFramebufferDrawBuffers(geoFbId, 5, attachments);

	if (glCheckNamedFramebufferStatus(geoFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("[Error] Framebuffer is not complete!");
	}

	renderer.addTexture(posTexId, GL_TEXTURE_2D, posTexUnit);
	renderer.addTexture(normTexId, GL_TEXTURE_2D, normTexUnit);
	renderer.addTexture(diffSpecTexId, GL_TEXTURE_2D, diffSpecTexUnit);
	renderer.addTexture(ssaoPosTexId, GL_TEXTURE_2D, ssaoPosTexUnit);
	renderer.addTexture(ssaoNormTexId, GL_TEXTURE_2D, ssaoNormTexUnit);
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
	renderer.setUniform1i(volumeProgramId, "ssaoSamp", blurTexUnit);
	volumeTexIds = { posTexId, normTexId, diffSpecTexId, blurTexId };

	renderer.setUniform2f(volumeProgramId, "screenDims", glm::vec2{ window.width, window.height });

	volumeModelData = std::vector<float>{};
	volumeModelData.reserve(lightCount * 16);
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		glm::mat4 model = glm::mat4{ 1.0f };
		float radius = getLightVolumeRadius(lights[i]);
		std::cout << radius << std::endl;
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
	const float sqrtDiscrim = std::sqrtf(kl * kl - 4.0f * kq * (kc - iMax * 256.0f / 0.05f));
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

void SSAO::prepareSSAOAssets()
{
	kernelSamples.clear();
	kernelSamples.reserve(kernelSampleCount * sizeof(glm::vec3));
	std::uniform_real_distribution<float> randomFloats{ 0.0f, 1.0f };
	for (uint32_t i = 0; i < kernelSampleCount; i++)
	{
		glm::vec3 sample(
			randomFloats(randomEngine) * 2.0f - 1.0f,
			randomFloats(randomEngine) * 2.0f - 1.0f,
			randomFloats(randomEngine)
		);

		sample = glm::normalize(sample);
		float scale = i / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		kernelSamples.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoiseDir{};
	for (uint32_t i = 0; i < noiseTexLen * noiseTexLen; i++)
	{
		glm::vec3 noise{
			randomFloats(randomEngine) * 2.0f - 1.0f,
			randomFloats(randomEngine) * 2.0f - 1.0f,
			0.0f
		};

		ssaoNoiseDir.push_back(noise);
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &noiseTexId);
	glTextureStorage2D(noiseTexId, 1, GL_RGB16F, noiseTexLen, noiseTexLen);
	glTextureSubImage2D(noiseTexId, 0, 0, 0, noiseTexLen, noiseTexLen, GL_RGB, GL_FLOAT, ssaoNoiseDir.data());
	glTextureParameteri(noiseTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(noiseTexId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	renderer.addTexture(noiseTexId, GL_TEXTURE_2D, noiseTexUnit);
}

void SSAO::prepareSSAO()
{
	prepareSSAOAssets();

	// AO buffer
	glCreateTextures(GL_TEXTURE_2D, 1, &ssaoTexId);
	glTextureStorage2D(ssaoTexId, 1, GL_R8, window.width, window.height);
	glTextureParameteri(ssaoTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ssaoTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	renderer.addTexture(ssaoTexId, GL_TEXTURE_2D, ssaoTexUnit);

	renderer.generateFramebuffer(ssaoFbId, {
		{ GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexId }
		});

	if (glCheckNamedFramebufferStatus(ssaoFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("[Error] Framebuffer is not complete!");
	}

	// Blur buffer
	glCreateTextures(GL_TEXTURE_2D, 1, &blurTexId);
	glTextureStorage2D(blurTexId, 1, GL_R8, window.width, window.height);
	glTextureParameteri(blurTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(blurTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	renderer.addTexture(blurTexId, GL_TEXTURE_2D, blurTexUnit);

	renderer.generateFramebuffer(blurFbId, {
		{ GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexId }
		});

	if (glCheckNamedFramebufferStatus(blurFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("[Error] Framebuffer is not complete!");
	}

	std::vector<float> ssaoVertices = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	std::vector<uint32_t> ssaoIndices = {
		0, 1, 2,
		3, 2, 1,
	};

	uint32_t vertBufId;
	renderer.generateVertexBuffer(vertBufId, ssaoVertices);

	uint32_t indexBufId;
	renderer.generateIndexBuffer(indexBufId, ssaoIndices);

	renderer.createVertexArray(ssaoVaoId, vertBufId, indexBufId, {
		{ 2, GL_FLOAT, 0 },
		{ 2, GL_FLOAT, 1 }, });

	renderer.generateProgram(ssaoProgramId, "Shaders/SSAO/SSAOVS.glsl", "Shaders/SSAO/SSAOFS.glsl");
	renderer.setUniform1i(ssaoProgramId, "posSamp", ssaoPosTexUnit);
	renderer.setUniform1i(ssaoProgramId, "normSamp", ssaoNormTexUnit);
	renderer.setUniform1i(ssaoProgramId, "texNoiseSamp", noiseTexUnit);
	renderer.setUniform3fv(ssaoProgramId, "samples", glm::value_ptr(kernelSamples[0]), kernelSampleCount);
	renderer.setUniform1i(ssaoProgramId, "sampleCount", kernelSampleCount);
	renderer.setUniform2f(ssaoProgramId, "noiseScale", glm::vec2{
		window.width / static_cast<float>(noiseTexLen), window.height / static_cast<float>(noiseTexLen) });
	ssaoTexIds = {
		ssaoPosTexId, ssaoNormTexId, noiseTexId
	};

	renderer.generateProgram(blurProgramId, "Shaders/SSAO/BlurVS.glsl", "Shaders/SSAO/BlurFS.glsl");
	renderer.setUniform1i(blurProgramId, "ssaoSamp", ssaoTexUnit);
	renderer.setUniform1f(blurProgramId, "noiseTexLen", static_cast<float>(noiseTexLen));
	blurTexIds = {
		ssaoTexId
	};
}

float SSAO::lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void SSAO::prepareHDR()
{
	uint32_t hdrTextureId;
	glCreateTextures(GL_TEXTURE_2D, 1, &hdrTextureId);
	glTextureStorage2D(hdrTextureId, 1, GL_RGBA16F, window.width, window.height);
	renderer.addTexture(hdrTextureId, GL_TEXTURE_2D, hdrTexUnit);

	uint32_t rbId;
	renderer.generateRenderbuffer(rbId, window.width, window.height);

	renderer.generateFramebuffer(hdrFbId, {
		{ GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTextureId },
		{ GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbId }, });

	renderer.generateProgram(hdrProgramId, "Shaders/HdrVS.glsl", "Shaders/HdrFS.glsl");
	renderer.setUniform1f(hdrProgramId, "exposure", 1.0f);
	renderer.setUniform1i(hdrProgramId, "screenTexture", hdrTexUnit);

	hdrTexIds = {hdrTextureId};
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

	renderer.bindFramebuffer(ssaoFbId);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.bindFramebuffer(blurFbId);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderer.bindFramebuffer(hdrFbId);
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
		renderer.prepareForDraw(ssaoFbId, ssaoProgramId, ssaoTexIds, ssaoVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_FRAMEBUFFER_SRGB);

		renderer.applyMvp(ssaoProgramId, "", "view", "projection");
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(ssaoProgramId, ssaoTexIds);
	}

	{
		renderer.prepareForDraw(blurFbId, blurProgramId, blurTexIds, ssaoVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_FRAMEBUFFER_SRGB);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(blurProgramId, ssaoTexIds);
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
		glDisable(GL_FRAMEBUFFER_SRGB);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(stencilProgramId, "", "view", "projection");
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, volumeModelSubs)), lightCount);

		renderer.unprepareForDraw(stencilProgramId, {});
	}

	{
		renderer.prepareForDraw(hdrFbId, volumeProgramId, volumeTexIds, volumeVaoId);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glStencilMask(0x00);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		// Gamma correction
		glEnable(GL_FRAMEBUFFER_SRGB);

		glBlitNamedFramebuffer(
			geoFbId, hdrFbId, 0, 0, window.width, window.height, 0, 0, window.width, window.height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
		);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(volumeProgramId, "", "view", "projection");
		renderer.setUniform3f(volumeProgramId, "viewPos", pos);
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, volumeModelSubs)), lightCount);

		renderer.unprepareForDraw(volumeProgramId, volumeTexIds);
	}

	{
		renderer.prepareForDraw(hdrFbId, lightProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_FRAMEBUFFER_SRGB);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(lightProgramId, "", "view", "projection");
		renderer.drawInstanced(60 * static_cast<uint32_t>(std::powf(4, lightModelSubs)), lightCount);

		renderer.unprepareForDraw(lightProgramId, {});
	}

	{
		renderer.prepareForDraw(0, hdrProgramId, hdrTexIds, ssaoVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_FRAMEBUFFER_SRGB);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(hdrProgramId, {});
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