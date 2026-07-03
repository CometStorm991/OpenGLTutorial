#include "PBR.hpp"

void PBR::prepare()
{
	prepareSpheres();

	renderer.prepareForRun();
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
	AttributeLayout tanAttrib{ 3, GL_FLOAT, 2 };

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);
	attribs.push_back(tanAttrib);

	renderer.generateVertexArray(sphereVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(sphereProgramId, "Shaders/SSAO/LightBoxVS.glsl", "Shaders/SSAO/LightBoxFS.glsl");

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/RustedIronPBRTextures/rustediron2_basecolor.png", true, GL_SRGB8, GL_TEXTURE_2D, 0);
	uint32_t texture1;
	renderer.generateResourceTexture2D(texture1, "Resources/RustedIronPBRTextures/rustediron2_normal.png", true, GL_RGB8, GL_TEXTURE_2D, 1);
	uint32_t texture2;
	renderer.generateResourceTexture2D(texture2, "Resources/RustedIronPBRTextures/rustediron2_metallic.png", true, GL_RGB8, GL_TEXTURE_2D, 2);
	uint32_t texture3;
	renderer.generateResourceTexture2D(texture3, "Resources/RustedIronPBRTextures/rustediron2_roughness.png", true, GL_RGB8, GL_TEXTURE_2D, 3);
	sphereTexIds = { texture0, texture1, texture2, texture3 };

	std::vector<float> modelData{};
	for (uint32_t i = 0; i < sphereCount; i++)
	{
		float x = static_cast<float>(i % 8 / 1);
		float y = static_cast<float>(i % 64 / 8);

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

	renderer.bindFramebuffer(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Camera camera = camController.getCamera();
	glm::mat4 view = camera.getView();
	glm::vec3 pos = camController.getCamera().pos;

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
		renderer.drawInstanced(3 * slices * 2 + 3 * 2 * (stacks - 2) * slices, sphereCount);

		renderer.unprepareForDraw(sphereProgramId, {});
	}

	{
		renderer.prepareForDraw(0, hdrProgramId, hdrTexIds, hdrVaoId);
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

bool PBR::shouldEnd()
{
	return window.getShouldClose();
}

void PBR::terminate()
{
	window.terminate();
}