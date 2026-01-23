#include "Instancing.hpp"

Instancing::Instancing()
{
}

void Instancing::prepare()
{
	prepareMetalCube();
	prepareSkybox();

	camController.setCameraPos(glm::vec3(0.0f, 0.0f, 0.0f));

	renderer.prepareForRun();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);
}

void Instancing::prepareMetalCube()
{
	std::vector<float> vertices;
	Cube::generatePN(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	AttributeLayout normAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);
	attribs.push_back(normAttrib);

	renderer.generateVertexArray(metalBoxVertexArrayId, vertexBuffer, 0, attribs);

	renderer.generateProgram(metalBoxProgramId, "Shaders/MetalBoxVS.glsl", "Shaders/MetalBoxFS.glsl");

	renderer.setUniform3f(metalBoxProgramId, "metal.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
	renderer.setUniform3f(metalBoxProgramId, "metal.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	renderer.setUniform3f(metalBoxProgramId, "metal.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	renderer.setUniform1f(metalBoxProgramId, "metal.shininess", 32.0f);
	renderer.setUniform1f(metalBoxProgramId, "metal.reflectance", 0.5f);

	glm::vec3 directionalLightDir = glm::vec3(0.2f, -0.5f, 0.3f);

	renderer.setUniform3f(metalBoxProgramId, "directionalLight.direction", directionalLightDir);
	renderer.setUniform3f(metalBoxProgramId, "directionalLight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
	renderer.setUniform3f(metalBoxProgramId, "directionalLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	renderer.setUniform3f(metalBoxProgramId, "directionalLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

	renderer.setUniform1i(metalBoxProgramId, "skybox", 0);
}

void Instancing::prepareSkybox()
{
	std::vector<std::string> textureFaces =
	{
		"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
	};
	for (uint32_t i = 0; i < textureFaces.size(); i++)
	{
		textureFaces[i] = "Resources/TutorialSkybox/" + textureFaces[i];
	}
	renderer.generateResourceTextureCubemap(skyboxTextureId, textureFaces, false, GL_TEXTURE_CUBE_MAP, 0);

	std::vector<float> skyboxVertices;
	Cube::generatePSkybox(skyboxVertices);

	uint32_t skyboxVertexBuffer;
	renderer.generateVertexBuffer(skyboxVertexBuffer, skyboxVertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = { posAttrib };

	renderer.generateVertexArray(skyboxVertexArrayId, skyboxVertexBuffer, 0, attribs);

	renderer.addTexture(skyboxTextureId, GL_TEXTURE_CUBE_MAP);

	renderer.generateProgram(skyboxProgramId, "Shaders/SkyboxVS.glsl", "Shaders/SkyboxFS.glsl");
	renderer.setUniform1i(skyboxProgramId, "skybox", 0);
}

void Instancing::run()
{
	renderer.prepareForFrame();
	glm::mat4 view = camController.getCamera().getView();
	glm::vec3 pos = camController.getCamera().pos;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		renderer.prepareForDraw(metalBoxProgramId, {}, metalBoxVertexArrayId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		//glEnable(GL_CULL_FACE);

		uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();

		glm::mat4 centerModel = glm::mat4(1.0f);
		//centerModel = glm::scale(centerModel, glm::vec3(10.0f, 10.0f, 10.0f));
		renderer.setUniformMatrix4fv(metalBoxProgramId, "centerModel", centerModel);

		glm::mat4 ringModel = glm::mat4(1.0f);
		renderer.setUniformMatrix4fv(metalBoxProgramId, "ringModel", ringModel);

		renderer.updateViewMatrix(view);
		renderer.applyMvp(metalBoxProgramId, "", "view", "projection");
		renderer.setUniform1f(metalBoxProgramId, "time", milliseconds / 1000.0f);
		renderer.setUniform3f(metalBoxProgramId, "viewPos", pos);
		renderer.drawInstanced(36, 1500);

		renderer.unprepareForDraw(metalBoxProgramId, {});
	}

	// -------- Skybox --------
	{
		renderer.prepareForDraw(skyboxProgramId, { skyboxTextureId }, skyboxVertexArrayId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);

		glm::mat4 model = glm::mat4(1.0f);
		renderer.updateModelMatrix(model);
		glm::mat4 newView = glm::mat4(glm::mat3(view));
		renderer.updateViewMatrix(newView);
		renderer.applyMvp(skyboxProgramId, "", "view", "projection");
		
		renderer.draw(36);

		renderer.unprepareForDraw(skyboxProgramId, { skyboxTextureId });
	}

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

bool Instancing::shouldEnd()
{
	return window.getShouldClose();
}
void Instancing::terminate()
{
	window.terminate();
}