#include "PBR.hpp"

PBR::PBR() {}

void PBR::prepare()
{
	prepareEnvMap();
	prepareEnvCubemap();
	prepareIrradiance();
	preparePrefiltered();
	prepareSkybox();
	prepareSpheres();
	prepareLights();
	prepareHDR();

	renderer.prepareForRun();
}

void PBR::prepareEnvMap()
{
	// Load image manually since no abstractions for HDR maps exists in Renderer
	stbi_set_flip_vertically_on_load(true);
	int width, height, channelCount;
	float* data = stbi_loadf("Resources/SeaviewHDRMap/relax_inn_seaview_suite_4k.hdr", &width, &height, &channelCount, 0);
	if (data)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &envTexId);
		glTextureStorage2D(envTexId, 1, GL_RGB16F, width, height);
		glTextureSubImage2D(envTexId, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

		glTextureParameteri(envTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(envTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(envTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(envTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "[Error] Failed to load Seaview texture" << std::endl;
	}

	renderer.addTexture(envTexId, GL_TEXTURE_2D, envTexUnit);
}

void PBR::prepareEnvCubemap()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &envCubemapTexId);
	glTextureStorage2D(envCubemapTexId, envCubemapMipLevels, GL_RGB16F, envCubemapLength, envCubemapLength);

	glTextureParameteri(envCubemapTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemapTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemapTexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(envCubemapTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(envCubemapTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	renderer.addTexture(envCubemapTexId, GL_TEXTURE_CUBE_MAP, envCubemapTexUnit);

	glCreateFramebuffers(1, &envCubemapFbId);

	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = renderer.addToData(vertices, Cube::fillP(), stride, 3);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(envCubemapVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(envCubemapProgramId, "Shaders/PBR/CaptureVS.glsl", "Shaders/PBR/CaptureFS.glsl");
	renderer.setUniform1i(envCubemapProgramId, "equirectMap", envTexUnit);
	renderer.setUniformMatrix4fv(envCubemapProgramId, "projection", captureProjection);

	envCubemapTexIds = { envTexId };

	for (uint32_t i = 0; i < captureViews.size(); i++)
	{
		glNamedFramebufferTextureLayer(envCubemapFbId, GL_COLOR_ATTACHMENT0, envCubemapTexId, 0, i);
		if (glCheckNamedFramebufferStatus(envCubemapFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "[Error] Framebuffer is not complete!" << std::endl;
		}

		glViewport(0, 0, envCubemapLength, envCubemapLength);

		renderer.bindFramebuffer(envCubemapFbId);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.prepareForDraw(envCubemapFbId, envCubemapProgramId, envCubemapTexIds, envCubemapVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDisable(GL_FRAMEBUFFER_SRGB);

		renderer.setUniformMatrix4fv(envCubemapProgramId, "view", captureViews[i]);
		renderer.draw(36);

		renderer.unprepareForDraw(envCubemapProgramId, envCubemapTexIds);
	}

	glGenerateTextureMipmap(envCubemapTexId);
}

void PBR::prepareIrradiance()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &irradTexId);
	glTextureStorage2D(irradTexId, 1, GL_RGB16F, irradCubemapLength, irradCubemapLength);

	glTextureParameteri(irradTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradTexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(irradTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(irradTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	renderer.addTexture(irradTexId, GL_TEXTURE_CUBE_MAP, irradTexUnit);

	renderer.generateProgram(irradProgramId, "Shaders/PBR/IrradianceVS.glsl", "Shaders/PBR/IrradianceFS.glsl");
	renderer.setUniform1i(irradProgramId, "environmentMap", envCubemapTexUnit);
	renderer.setUniformMatrix4fv(irradProgramId, "projection", captureProjection);

	irradTexIds = { envCubemapTexId };

	for (uint32_t i = 0; i < captureViews.size(); i++)
	{
		glNamedFramebufferTextureLayer(envCubemapFbId, GL_COLOR_ATTACHMENT0, irradTexId, 0, i);
		if (glCheckNamedFramebufferStatus(envCubemapFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "[Error] Framebuffer is not complete!" << std::endl;
		}

		glViewport(0, 0, irradCubemapLength, irradCubemapLength);

		renderer.bindFramebuffer(envCubemapFbId);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.prepareForDraw(envCubemapFbId, irradProgramId, irradTexIds, envCubemapVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDisable(GL_FRAMEBUFFER_SRGB);

		renderer.setUniformMatrix4fv(irradProgramId, "view", captureViews[i]);
		renderer.draw(36);

		renderer.unprepareForDraw(irradProgramId, irradTexIds);
	}
}

void PBR::preparePrefiltered()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &prefTexId);
	glTextureStorage2D(prefTexId, prefMipLevels, GL_RGBA16F, prefCubemapLength, prefCubemapLength);

	glTextureParameteri(prefTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(prefTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(prefTexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(prefTexId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(prefTexId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	renderer.addTexture(prefTexId, GL_TEXTURE_CUBE_MAP, prefTexUnit);

	renderer.generateProgram(prefProgramId, "Shaders/PBR/PrefilteredVS.glsl", "Shaders/PBR/PrefilteredFS.glsl");
	renderer.setUniform1i(prefProgramId, "environmentMap", envCubemapTexUnit);
	renderer.setUniform1f(prefProgramId, "resolution", envCubemapLength);
	renderer.setUniformMatrix4fv(prefProgramId, "projection", captureProjection);

	prefTexIds = { envCubemapTexId };

	for (uint32_t i = 0; i < prefMipLevels; i++)
	{
		uint32_t mipmapLength = prefCubemapLength >> i;
		float roughness = i / static_cast<float>(prefMipLevels - 1);
		renderer.setUniform1f(prefProgramId, "roughness", roughness);

		glViewport(0, 0, mipmapLength, mipmapLength);

		for (uint32_t j = 0; j < captureViews.size(); j++)
		{
			glNamedFramebufferTextureLayer(envCubemapFbId, GL_COLOR_ATTACHMENT0, prefTexId, i, j);
			if (glCheckNamedFramebufferStatus(envCubemapFbId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cerr << "[Error] Framebuffer is not complete!" << std::endl;
			}

			renderer.bindFramebuffer(envCubemapFbId);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			
			renderer.prepareForDraw(envCubemapFbId, prefProgramId, prefTexIds, envCubemapVaoId);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
			glDisable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glDisable(GL_FRAMEBUFFER_SRGB);

			renderer.setUniformMatrix4fv(prefProgramId, "view", captureViews[j]);
			renderer.draw(36);

			renderer.unprepareForDraw(prefProgramId, prefTexIds);
		}
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void PBR::prepareSkybox()
{
	std::vector<float> skyboxVertices;
	Cube::generatePSkybox(skyboxVertices);

	uint32_t skyboxVertexBuffer;
	renderer.generateVertexBuffer(skyboxVertexBuffer, skyboxVertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = { posAttrib };

	renderer.generateVertexArray(skyboxVaoId, skyboxVertexBuffer, 0, attribs);

	renderer.generateProgram(skyboxProgramId, "Shaders/PBR/SkyboxVS.glsl", "Shaders/PBR/SkyboxFS.glsl");
	//renderer.setUniform1i(skyboxProgramId, "skybox", envCubemapTexUnit);
	//skyboxTexIds = { envCubemapTexId };
	
	//renderer.setUniform1i(skyboxProgramId, "skybox", irradTexUnit);
	//skyboxTexIds = { irradTexId };

	renderer.setUniform1i(skyboxProgramId, "skybox", prefTexUnit);
	skyboxTexIds = { prefTexId };
}

void PBR::prepareSpheres()
{
	std::vector<float> vertices;
	uint32_t stride = 0;

	stride = renderer.addToData(vertices, UVSphere::fillP(slices, stacks), stride, 3);
	stride = renderer.addToData(vertices, UVSphere::fillN(slices, stacks), stride, 3);
	stride = renderer.addToData(vertices, UVSphere::fillT(slices, stacks), stride, 2);
	stride = renderer.addToData(vertices, UVSphere::fillTan(slices, stacks), stride, 3);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib{ 3, GL_FLOAT, 0 };
	AttributeLayout normAttrib{ 3, GL_FLOAT, 1 };
	AttributeLayout texAttrib{ 2, GL_FLOAT, 2 };
	AttributeLayout tanAttrib{ 3, GL_FLOAT, 3 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);
	attribs.push_back(tanAttrib);

	renderer.generateVertexArray(sphereVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(sphereProgramId, "Shaders/PBR/SphereVS.glsl", "Shaders/PBR/SphereFS.glsl");
	renderer.setUniform1i(sphereProgramId, "albedoSamp", 0);
	renderer.setUniform1i(sphereProgramId, "normSamp", 1);
	renderer.setUniform1i(sphereProgramId, "metallicSamp", 2);
	renderer.setUniform1i(sphereProgramId, "roughnessSamp", 3);
	renderer.setUniform1i(sphereProgramId, "irradianceSamp", irradTexUnit);

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/RustedIronPBRTextures/rustediron2_basecolor.png", true, GL_SRGB8, GL_TEXTURE_2D, 0);
	uint32_t texture1;
	renderer.generateResourceTexture2D(texture1, "Resources/RustedIronPBRTextures/rustediron2_normal.png", true, GL_RGB8, GL_TEXTURE_2D, 1);
	uint32_t texture2;
	renderer.generateResourceTexture2D(texture2, "Resources/RustedIronPBRTextures/rustediron2_metallic.png", true, GL_R8, GL_TEXTURE_2D, 2);
	uint32_t texture3;
	renderer.generateResourceTexture2D(texture3, "Resources/RustedIronPBRTextures/rustediron2_roughness.png", true, GL_R8, GL_TEXTURE_2D, 3);
	sphereTexIds = { texture0, texture1, texture2, texture3, irradTexId };

	std::vector<float> modelData{};
	for (uint32_t i = 0; i < sphereCount; i++)
	{
		float x = static_cast<float>(i % 8 / 1) * 3.0f;
		float y = static_cast<float>(i % 64 / 8) * 3.0f;

		glm::mat4 modelMat{ 1.0f };
		modelMat = glm::translate(modelMat, glm::vec3{ x, y, 0.0f });

		const float* modelPtr = glm::value_ptr(modelMat);
		modelData.insert(modelData.end(), modelPtr, modelPtr + 16);
	}

	std::vector<float> instData;
	uint32_t instStride = 0;

	instStride = renderer.addToData(instData, modelData, instStride, 16);

	uint32_t instBuffer;
	renderer.generateVertexBuffer(instBuffer, instData);

	std::vector<AttributeLayout> instAttribs = {
		{4, GL_FLOAT, 4},
		{4, GL_FLOAT, 5},
		{4, GL_FLOAT, 6},
		{4, GL_FLOAT, 7}
	};

	renderer.addInstToVertexArray(sphereVaoId, instBuffer, instAttribs);
}

void PBR::prepareLights()
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
			0.5f, 0.5f, 0.5f
		};

		float posX = (i % 8 / 1) * 3.0f;
		float posY = (i % 64 / 8) * 3.0f;
		float posZ = (i % 256 / 64) * 3.0f + 10.0f;

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
		float linear = 0.7f;
		float quadratic = 1.8f;

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

void PBR::prepareHDR()
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

	std::vector<float> hdrVertices = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	std::vector<uint32_t> hdrIndices = {
		0, 1, 2,
		3, 2, 1,
	};

	uint32_t vertBufId;
	renderer.generateVertexBuffer(vertBufId, hdrVertices);

	uint32_t indexBufId;
	renderer.generateIndexBuffer(indexBufId, hdrIndices);

	renderer.createVertexArray(hdrVaoId, vertBufId, indexBufId, {
		{ 2, GL_FLOAT, 0 },
		{ 2, GL_FLOAT, 1 }, });

	renderer.generateProgram(hdrProgramId, "Shaders/PBR/HdrVS.glsl", "Shaders/PBR/HdrFS.glsl");
	renderer.setUniform1f(hdrProgramId, "exposure", 1.0f);
	renderer.setUniform1i(hdrProgramId, "screenTexture", hdrTexUnit);

	hdrTexIds = { hdrTextureId };
}

void PBR::run()
{
	renderer.prepareForFrame();

	glViewport(0, 0, window.width, window.height);

	renderer.bindFramebuffer(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.bindFramebuffer(hdrFbId);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Camera camera = camController.getCamera();
	glm::mat4 view = camera.getView();
	glm::vec3 pos = camController.getCamera().pos;
	float exposure = camController.getCamera().exposure;

	{
		renderer.prepareForDraw(hdrFbId, sphereProgramId, sphereTexIds, sphereVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_FRAMEBUFFER_SRGB);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(sphereProgramId, "", "view", "projection");
		renderer.setUniform3f(sphereProgramId, "viewPos", camera.pos);
		renderer.drawInstanced(3 * slices * 2 + 3 * 2 * (stacks - 2) * slices, sphereCount);

		renderer.unprepareForDraw(sphereProgramId, {});
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
		renderer.prepareForDraw(0, hdrProgramId, hdrTexIds, hdrVaoId);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_FRAMEBUFFER_SRGB);

		renderer.setUniform1f(hdrProgramId, "exposure", exposure);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(hdrProgramId, {});
	}

	{
		renderer.prepareForDraw(0, skyboxProgramId, skyboxTexIds, skyboxVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_FRAMEBUFFER_SRGB);

		glBlitNamedFramebuffer(
			hdrFbId, 0, 0, 0, window.width, window.height, 0, 0, window.width, window.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);

		renderer.updateViewMatrix(glm::mat4{ glm::mat3{ view } });
		renderer.applyMvp(skyboxProgramId, "", "view", "projection");
		renderer.draw(36);

		renderer.unprepareForDraw(skyboxProgramId, skyboxTexIds);
	}

	renderer.unprepareForFrame(exposure);

	window.updateGLFW();
	camController.updateCameraMicroseconds(window.getInputState(), renderer.getFrameTimeMicroseconds());
	window.updateMouse();
}

bool PBR::shouldEnd()
{
	return window.getShouldClose();
}

void PBR::terminate()
{
	window.terminate();
}