#include "AdvancedLighting.hpp"

AdvancedLighting::AdvancedLighting()
{

}

void AdvancedLighting::prepare()
{
	prepareFloor();
	prepareLight();

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
	textureIds.clear();
	textureIds.push_back(texture0);

	renderer.generateProgram(programId, "Shaders/TexturedObjectVS.glsl", "Shaders/TexturedObjectFS.glsl");

	renderer.setUniform1i(programId, "material.diffuse", 0);
	renderer.setUniform1f(programId, "material.specular", 1.0f);
	renderer.setUniform1f(programId, "material.shininess", 4.0f);

	renderer.setUniform3f(programId, "pointLight.position", pointLightPos);
	renderer.setUniform3f(programId, "pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	renderer.setUniform3f(programId, "pointLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	renderer.setUniform3f(programId, "pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	renderer.setUniform1f(programId, "pointLight.constant", 1.0f);
	renderer.setUniform1f(programId, "pointLight.linear", 0.07f);
	renderer.setUniform1f(programId, "pointLight.quadratic", 0.017f);

	floorModel = glm::scale(floorModel, glm::vec3(50.0f, 1.0f, 50.0f));
	
}

void AdvancedLighting::prepareLight()
{
	std::vector<float> vertices;
	Cube::generatePNT(vertices);

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, vertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
	attribs.push_back(posAttrib);

	renderer.generateVertexArray(lightVaoId, vertexBuffer, 0, attribs);

	renderer.generateProgram(lightProgramId, "Shaders/ALLightVS.glsl", "Shaders/ALLightFS.glsl");

	lightModel = glm::scale(lightModel, glm::vec3(0.5f, 0.5f, 0.5f));
	lightModel = glm::translate(lightModel, pointLightPos);
}

void AdvancedLighting::run()
{
	renderer.prepareForFrame();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camController.getCamera().getView();
	glm::vec3 pos = camController.getCamera().pos;

	{
		renderer.prepareForDraw(programId, textureIds, floorVaoId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		renderer.updateModelMatrix(floorModel);
		renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(floorModel)));
		renderer.updateViewMatrix(view);
		renderer.applyMvp(programId, "model", "view", "projection");
		renderer.setUniform3f(programId, "viewPos", pos);
		renderer.draw(36);

		renderer.unprepareForDraw(programId, textureIds);
	}

	{
		renderer.prepareForDraw(lightProgramId, {}, floorVaoId);
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

	renderer.unprepareForFrame();

	window.updateGLFW();
	camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

bool AdvancedLighting::shouldEnd()
{
	return window.getShouldClose();
}
void AdvancedLighting::terminate()
{
	window.terminate();
}