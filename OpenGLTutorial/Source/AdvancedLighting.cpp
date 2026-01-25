#include "AdvancedLighting.hpp"

AdvancedLighting::AdvancedLighting()
{

}

void AdvancedLighting::prepare()
{
	prepareFloor();
	prepareBoxes();
	prepareLight();
	prepareShadows();
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

void AdvancedLighting::prepareFloor()
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);

	uint32_t floatsPerVertex = 8;

	// If vertex float count is not divisible by floatsPerVertex
	if (std::abs((float)(vertices.size() / floatsPerVertex) - ((float)vertices.size() / floatsPerVertex)) > 0.001f)
	{
		std::cerr << "[Error]: Vertex float count " << vertices.size() << " is not divisible by floatsPerVertex " << floatsPerVertex << std::endl;
	}

	uint32_t vertexCount = vertices.size() / floatsPerVertex;
	for (uint32_t i = 0; i < vertexCount; i++)
	{
		vertices[i * floatsPerVertex + 6] *= 10.0f;
		vertices[i * floatsPerVertex + 7] *= 10.0f;
	}

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

	uint32_t texture0;
	renderer.generateResourceTexture2D(texture0, "Resources/TutorialWood.png", true, GL_SRGB8, GL_TEXTURE_2D, 0);
	floorTextureIds.clear();
	floorTextureIds.push_back(texture0);

	renderer.generateProgram(floorProgramId, "Shaders/TexturedObjectVS.glsl", "Shaders/TexturedObjectFS.glsl");

	renderer.setUniform1i(floorProgramId, "material.diffuse", 0);
	renderer.setUniform1f(floorProgramId, "material.specular", 1.0f);
	renderer.setUniform1f(floorProgramId, "material.shininess", 4.0f);
	applyLightUniforms(floorProgramId);

	floorModel = glm::scale(floorModel, glm::vec3(50.0f, 1.0f, 50.0f));
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

	applyLightUniforms(boxesProgramId);
}

void AdvancedLighting::prepareLight()
{
	std::vector<float> vertices;
	Cube::generateP(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(lightVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(lightProgramId, "Shaders/ALLightVS.glsl", "Shaders/ALLightFS.glsl");

	lightModel = glm::translate(lightModel, ptLightPos);
	//lightModel = glm::scale(lightModel, glm::vec3(0.5f, 0.5f, 0.5f));
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
		renderer.prepareForDraw(depthMapFbId, depthMapProgramId, {}, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateModelMatrix(floorModel);
		renderer.applyMvp(depthMapProgramId, "model", "", "");
		renderer.draw(36);

		renderer.unprepareForDraw(depthMapProgramId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(depthMapFbId, depthMapProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateModelMatrix(lightModel);
		renderer.applyMvp(depthMapProgramId, "model", "", "");
		renderer.draw(36);

		renderer.unprepareForDraw(depthMapProgramId, {});
	}

	{
		renderer.prepareForDraw(depthMapFbId, depthMapInstancedProgId, {}, boxesVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.drawInstanced(36, boxesCount);

		renderer.unprepareForDraw(boxesProgramId, boxesTextureIds);
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

		renderer.updateModelMatrix(floorModel);
		renderer.setUniformMatrix4fv(floorProgramId, "normalMatrix", glm::transpose(glm::inverse(floorModel)));
		renderer.updateViewMatrix(view);
		renderer.applyMvp(floorProgramId, "model", "view", "projection");
		renderer.setUniform3f(floorProgramId, "viewPos", pos);
		renderer.draw(36);

		renderer.unprepareForDraw(floorProgramId, floorTextureIds);
	}

	{
		renderer.prepareForDraw(lightProgramId, {}, lightVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateModelMatrix(lightModel);
		renderer.updateViewMatrix(view);
		renderer.applyMvp(lightProgramId, "model", "view", "projection");
		renderer.setUniform3f(lightProgramId, "viewPos", pos);
		renderer.draw(36);

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