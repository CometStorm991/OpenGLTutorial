#include "AdvancedLighting.hpp"

AdvancedLighting::AdvancedLighting()
{

}

void AdvancedLighting::prepare()
{
	// Floors
	glm::mat4 floorModel = glm::mat4(1.0f);
	floorModel = glm::scale(floorModel, glm::vec3{ 50.0f, 1.0f, 50.0f });
	std::vector<Floor> floors = {
		{10.0f, floorModel}
	};
	floorCount++;
	buildFloorCube(floors);

	prepareFloor(floors);

	// Boxes
	prepareBoxes();

	// Lights
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, glm::vec3{0.0f, 5.0f, 0.0f});
	lightModel = glm::translate(lightModel, glm::vec3{ -0.5f, -0.5f, -0.5f });
	lightModel = glm::scale(lightModel, glm::vec3{ 0.5f, 0.5f, 0.5f });
	std::vector<Light> lights = {
		{
			{ 1.0f, 1.0f, 1.0f },
			lightModel,
			{ 0.0f, 5.0f, 0.0f },
			{ 0.2f, 0.2f, 0.2f },
			{ 0.5f, 0.5f, 0.5f },
			{ 1.0f, 1.0f, 1.0f },
			1.0f, 0.07f, 0.017f
		}
	};
	lightCount = 1;

	class GPULight {
	public:
		glm::vec4 pos;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 attentuation;
	};
	std::vector<GPULight> gpuLights;

	uint32_t ssbId;
	uint32_t lightSize = 5 * sizeof(glm::vec4);
	uint32_t ssbSize = lights.size() * lightSize;
	glCreateBuffers(1, &ssbId);
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
	glNamedBufferStorage(ssbId, ssbSize, gpuLights.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbId);

	prepareLight(lights);

	prepareShadows();
	prepareCubemapShadows();
	renderer.setUniformMatrix4fv(boxesProgramId, "lightSpaceMat", lightSpaceMat);
	renderer.setUniform1i(boxesProgramId, "depthMap", depthMapTexUnit);
	renderer.setUniformMatrix4fv(floorProgramId, "lightSpaceMat", lightSpaceMat);
	renderer.setUniform1i(floorProgramId, "depthMap", depthMapTexUnit);
	boxesTextureIds.push_back(depthMapTexId);

	camController.setCameraPos(glm::vec3(-5.0f, 2.0f, 0.0f));
	camController.setCameraOrientation(0.0f, 0.0f);

	renderer.prepareForRun();

	// [Warning]: Will break with deferred shading
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	// Gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void AdvancedLighting::buildFloorCube(std::vector<Floor>& floors)
{
	float floorCubeWidth = floorCubeSize * 2.0f;
	float floorCubeTexScale = floorCubeSize * 2.0f * (10.0f / 50.0f);

	glm::mat4 top = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	top = glm::translate(top, glm::vec3(0.0f, floorCubeSize, 0.0f));
	top = glm::scale(top, glm::vec3(floorCubeWidth, floorCubeThickness, floorCubeWidth));

	glm::mat4 bottom = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	bottom = glm::translate(bottom, glm::vec3(0.0f, -floorCubeSize, 0.0f));
	bottom = glm::scale(bottom, glm::vec3(floorCubeWidth, floorCubeThickness, floorCubeWidth));

	glm::mat4 right = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	right = glm::translate(right, glm::vec3(floorCubeSize, 0.0f, 0.0f));
	right = glm::scale(right, glm::vec3(floorCubeThickness, floorCubeWidth, floorCubeWidth));

	glm::mat4 left = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	left = glm::translate(left, glm::vec3(-floorCubeSize, 0.0f, 0.0f));
	left = glm::scale(left, glm::vec3(floorCubeThickness, floorCubeWidth, floorCubeWidth));

	glm::mat4 front = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	front = glm::translate(front, glm::vec3(0.0f, 0.0f, floorCubeSize));
	front = glm::scale(front, glm::vec3(floorCubeWidth, floorCubeWidth, floorCubeThickness));

	glm::mat4 back = glm::translate(glm::mat4{ 1.0f }, floorCubeOffset);
	back = glm::translate(back, glm::vec3(0.0f, 0.0f, -floorCubeSize));
	back = glm::scale(back, glm::vec3(floorCubeWidth, floorCubeWidth, floorCubeThickness));

	floors.emplace_back(floorCubeTexScale, top);
	floors.emplace_back(floorCubeTexScale, bottom);
	floors.emplace_back(floorCubeTexScale, right);
	floors.emplace_back(floorCubeTexScale, left);
	floors.emplace_back(floorCubeTexScale, front);
	floors.emplace_back(floorCubeTexScale, back);

	floorCount += 6;
}

void AdvancedLighting::prepareFloor(const std::vector<Floor>& floors)
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout normAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);

	renderer.generateVertexArray(floorVaoId, vertexBuffer, 0, attribs);

	// Array of structures to structure of arrays

	uint32_t instBuffer;
	uint32_t instBufferSize = floors.size() * sizeof(Floor);
	glCreateBuffers(1, &instBuffer);
	glNamedBufferStorage(instBuffer, instBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	for (uint32_t i = 0; i < floors.size(); i++)
	{
		glNamedBufferSubData(instBuffer, i * sizeof(Floor), sizeof(glm::mat4), glm::value_ptr(floors[i].modelMat));
		glNamedBufferSubData(instBuffer, i * sizeof(Floor) + sizeof(glm::mat4), sizeof(float), &(floors[i].texScale));
	}
	glVertexArrayVertexBuffer(floorVaoId, 1, instBuffer, 0, sizeof(Floor));

	// model matrix (aModel)
	uint32_t startAttrib = 3;
	for (uint32_t i = 0; i < 4; i++)
	{
		uint32_t attrib = startAttrib + i;

		glEnableVertexArrayAttrib(floorVaoId, attrib);
		glVertexArrayAttribFormat(floorVaoId, attrib, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
		glVertexArrayAttribBinding(floorVaoId, attrib, 1);
	}

	// Texture scaling (aTexScale)
	startAttrib = 7;
	glEnableVertexArrayAttrib(floorVaoId, startAttrib);
	glVertexArrayAttribFormat(floorVaoId, startAttrib, 1, GL_FLOAT, GL_FALSE, sizeof(glm::mat4));
	glVertexArrayAttribBinding(floorVaoId, startAttrib, 1);

	glVertexArrayBindingDivisor(floorVaoId, 1, 1);

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/TutorialWood.png", true, GL_SRGB8, GL_TEXTURE_2D, 0);
	floorTextureIds.clear();
	floorTextureIds.push_back(texture0);

	renderer.generateProgram(floorProgramId, "Shaders/TexturedObjectVS.glsl", "Shaders/TexturedObjectFS.glsl");

	renderer.setUniform1i(floorProgramId, "material.diffuse", 0);
	renderer.setUniform1f(floorProgramId, "material.specular", 1.0f);
	renderer.setUniform1f(floorProgramId, "material.shininess", 4.0f);
	//applyLightUniforms(floorProgramId);
}

void AdvancedLighting::prepareBoxes()
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout normAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);
	attribs.push_back(texAttrib);

	renderer.generateVertexArray(boxesVaoId, vertexBuffer, 0, attribs);

	glm::mat4 model1 = glm::translate(glm::mat4{ 1.0f }, glm::vec3(0.0f, 2.0f, 5.0f));
	glm::mat4 model2 = glm::mat4{ 1.0f };
	model2 = glm::translate(model2, glm::vec3(0.0f, 2.0f, -5.0f));
	model2 = glm::rotate(model2, 3.14f / 4.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 model3 = glm::translate(glm::mat4{ 1.0f }, glm::vec3(0.0f, 1.0f, -8.0f));
	std::vector<glm::mat4> models{ model1, model2, model3 };
	glCreateBuffers(1, &vertexBuffer);
	glNamedBufferStorage(vertexBuffer, models.size() * sizeof(glm::mat4), models.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayVertexBuffer(boxesVaoId, 1, vertexBuffer, 0, sizeof(glm::mat4));

	uint32_t startAttrib = 3;
	for (uint32_t i = 0; i < 4; i++)
	{
		uint32_t attrib = startAttrib + i;

		glEnableVertexArrayAttrib(boxesVaoId, attrib);
		glVertexArrayAttribFormat(boxesVaoId, attrib, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
		glVertexArrayAttribBinding(boxesVaoId, attrib, 1);
	}

	glVertexArrayBindingDivisor(boxesVaoId, 1, 1);

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/TutorialDiffuseMap.png", true, GL_TEXTURE_2D, 0);
	uint32_t texture1;
	renderer.generateResourceTexture2D(texture1, "Resources/TutorialSpecularMap.png", true, GL_TEXTURE_2D, 1);
	boxesTextureIds.clear();
	boxesTextureIds.push_back(texture0);
	boxesTextureIds.push_back(texture1);

	renderer.generateProgram(boxesProgramId, "Shaders/TexDiffSpecVS.glsl", "Shaders/TexDiffSpecFS.glsl");

	renderer.setUniform1i(boxesProgramId, "material.diffuse", materialDiffuseTexUnit);
	renderer.setUniform1i(boxesProgramId, "material.specular", materialSpecularTexUnit);
	renderer.setUniform1f(boxesProgramId, "material.shininess", 4.0f);

	//applyLightUniforms(boxesProgramId);
}

void AdvancedLighting::prepareLight(const std::vector<Light>& lights)
{
	std::vector<float> vertices;
	Cube::generateP(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(lightVaoId, vertexBuffer, 0, attribs);

	uint32_t instBuffer;
	uint32_t instSize = sizeof(glm::mat4) + sizeof(glm::vec3);
	uint32_t instBufferSize = lights.size() * instSize;
	glCreateBuffers(1, &instBuffer);
	glNamedBufferStorage(instBuffer, instBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
	for (uint32_t i = 0; i < lights.size(); i++)
	{
		glNamedBufferSubData(instBuffer, i * instSize, sizeof(glm::mat4), glm::value_ptr(lights[i].modelMat));
		glNamedBufferSubData(instBuffer, i * instSize + sizeof(glm::mat4), sizeof(glm::vec3), &(lights[i].color));
	}
	glVertexArrayVertexBuffer(lightVaoId, 1, instBuffer, 0, instSize);

	uint32_t startAttrib = 3;
	for (uint32_t i = 0; i < 4; i++)
	{
		uint32_t attrib = startAttrib + i;

		glEnableVertexArrayAttrib(lightVaoId, attrib);
		glVertexArrayAttribFormat(lightVaoId, attrib, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
		glVertexArrayAttribBinding(lightVaoId, attrib, 1);
	}

	startAttrib = 7;
	glEnableVertexArrayAttrib(lightVaoId, startAttrib);
	glVertexArrayAttribFormat(lightVaoId, startAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat4));
	glVertexArrayAttribBinding(lightVaoId, startAttrib, 1);

	glVertexArrayBindingDivisor(lightVaoId, 1, 1);

	renderer.generateProgram(lightProgramId, "Shaders/ALLightVS.glsl", "Shaders/ALLightFS.glsl");
}

void AdvancedLighting::prepareShadows()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &depthMapTexId);
	glTextureStorage2D(depthMapTexId, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
	glTextureParameteri(depthMapTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthMapTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthMapTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(depthMapTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(depthMapTexId, GL_TEXTURE_BORDER_COLOR, borderColor);
	renderer.addTexture(depthMapTexId, GL_TEXTURE_2D, depthMapTexUnit);

	glCreateFramebuffers(1, &depthMapFbId);
	glNamedFramebufferTexture(depthMapFbId, GL_DEPTH_ATTACHMENT, depthMapTexId, 0);
	glNamedFramebufferDrawBuffer(depthMapFbId, GL_NONE);
	glNamedFramebufferReadBuffer(depthMapFbId, GL_NONE);

	renderer.generateProgram(depthMapProgramId, "Shaders/DepthMapVS.glsl", "Shaders/DepthMapFS.glsl");
	renderer.generateProgram(depthMapInstancedProgId, "Shaders/DepthMapInstancedVS.glsl", "Shaders/DepthMapInstancedFS.glsl");

	glm::mat4 lightView = glm::lookAt(
		ptLightPos,
		glm::vec3(0.0f, 2.0f, -5.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	float nearPlane = 1.0f, farPlane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	lightSpaceMat = lightProjection * lightView;
	renderer.setUniformMatrix4fv(depthMapProgramId, "lightSpaceMat", lightSpaceMat);
	renderer.setUniformMatrix4fv(depthMapInstancedProgId, "lightSpaceMat", lightSpaceMat);

	std::vector<float> quadVertices =
	{
		 -1.0f,  0.6f, 0.0f, 0.0f,
		 -0.6f,  0.6f, 1.0f, 0.0f,
		 -1.0f,  1.0f, 0.0f, 1.0f,
		 -0.6f,  1.0f, 1.0f, 1.0f,
	};
	std::vector<uint32_t> quadIndices =
	{
		0, 1, 2,
		3, 2, 1,
	};

	uint32_t quadVertexBuffer;
	renderer.generateVertexBuffer(quadVertexBuffer, quadVertices);
	uint32_t quadIndexBuffer;
	renderer.generateIndexBuffer(quadIndexBuffer, quadIndices);
	AttributeLayout quadPosAttrib = AttributeLayout(2, GL_FLOAT);
	AttributeLayout quadTexAttrib = AttributeLayout(2, GL_FLOAT);
	std::vector<AttributeLayout> quadAttribs =
	{
		quadPosAttrib, quadTexAttrib
	};

	renderer.generateVertexArray(quadVaoId, quadVertexBuffer, quadIndexBuffer, quadAttribs);

	renderer.generateProgram(quadProgramId, "Shaders/QuadVS.glsl", "Shaders/QuadFS.glsl");
	renderer.setUniform1i(quadProgramId, "screenTexture", depthMapTexUnit);
}

void AdvancedLighting::prepareCubemapShadows()
{
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &depthCubemapTexId);
	glTextureStorage2D(depthCubemapTexId, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthCubemapTexId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	renderer.addTexture(depthCubemapTexId, GL_TEXTURE_CUBE_MAP, depthCubemapTexUnit);

	glCreateFramebuffers(1, &depthCubemapFbId);
	glNamedFramebufferTexture(depthCubemapFbId, GL_DEPTH_ATTACHMENT, depthCubemapTexId, 0);
	glNamedFramebufferDrawBuffer(depthCubemapFbId, GL_NONE);
	glNamedFramebufferReadBuffer(depthCubemapFbId, GL_NONE);
}

void AdvancedLighting::applyLightUniforms(uint32_t programId)
{
	renderer.setUniform3f(programId, "pointLight.position", ptLightPos);
	renderer.setUniform3f(programId, "pointLight.ambient", ptLightAmbient);
	renderer.setUniform3f(programId, "pointLight.diffuse", ptLightDiffuse);
	renderer.setUniform3f(programId, "pointLight.specular", ptLightSpecular);
	renderer.setUniform1f(programId, "pointLight.constant", ptLightConstant);
	renderer.setUniform1f(programId, "pointLight.linear", ptLightLinear);
	renderer.setUniform1f(programId, "pointLight.quadratic", ptLightQuadratic);
}

void AdvancedLighting::run()
{
	renderer.prepareForFrame();

	// Clears framebuffers
	renderer.bindFramebuffer(0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer.bindFramebuffer(depthMapFbId);
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	renderShadowMap();
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	renderScene();

	glViewport(0, 0, SCREEN_HEIGHT, SCREEN_HEIGHT);
	{
		renderer.prepareForDraw(quadProgramId, { depthMapTexId }, quadVaoId);
		glDisable(GL_DEPTH_TEST);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		renderer.unprepareForDraw(quadProgramId, { depthMapTexId });
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

void AdvancedLighting::renderShadowMap()
{
	{
		renderer.prepareForDraw(depthMapFbId, depthMapInstancedProgId, {}, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, floorCount);

		renderer.unprepareForDraw(depthMapInstancedProgId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(depthMapFbId, depthMapInstancedProgId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, lightCount);

		renderer.unprepareForDraw(depthMapInstancedProgId, {});
	}

	{
		renderer.prepareForDraw(depthMapFbId, depthMapInstancedProgId, {}, boxesVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, boxesCount);

		renderer.unprepareForDraw(depthMapInstancedProgId, boxesTextureIds);
	}
}

void AdvancedLighting::renderShadowCubemap()
{
	{
		renderer.prepareForDraw(depthCubemapFbId, depthMapInstancedProgId, {}, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, floorCount);

		renderer.unprepareForDraw(depthMapInstancedProgId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(depthCubemapFbId, depthMapInstancedProgId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, lightCount);

		renderer.unprepareForDraw(depthMapInstancedProgId, {});
	}

	{
		renderer.prepareForDraw(depthCubemapFbId, depthCubemapProgId, {}, boxesVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, boxesCount);

		renderer.unprepareForDraw(depthCubemapProgId, boxesTextureIds);
	}
}

void AdvancedLighting::renderScene()
{
	glm::mat4 view = camController.getCamera().getView();
	glm::vec3 pos = camController.getCamera().pos;

	{
		renderer.prepareForDraw(floorProgramId, floorTextureIds, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(floorProgramId, "", "view", "projection");
		renderer.setUniform3f(floorProgramId, "viewPos", pos);
		renderer.drawInstanced(36, floorCount);

		renderer.unprepareForDraw(floorProgramId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(lightProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(lightProgramId, "", "view", "projection");
		renderer.setUniform3f(lightProgramId, "viewPos", pos);
		renderer.drawInstanced(36, lightCount);

		renderer.unprepareForDraw(lightProgramId, {});
	}

	{
		renderer.prepareForDraw(boxesProgramId, boxesTextureIds, boxesVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(boxesProgramId, "", "view", "projection");
		renderer.setUniform3f(boxesProgramId, "viewPos", pos);
		renderer.drawInstanced(36, boxesCount);

		renderer.unprepareForDraw(boxesProgramId, boxesTextureIds);
	}
}

bool AdvancedLighting::shouldEnd()
{
	return window.getShouldClose();
}
void AdvancedLighting::terminate()
{
	window.terminate();
}