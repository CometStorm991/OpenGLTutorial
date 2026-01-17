#include "Skybox.hpp"

Skybox::Skybox()
	: renderer(Renderer(camera))
{

}

void Skybox::init()
{
	renderer.init();
}

void Skybox::prepare()
{
	// renderer.generateTexture(skyboxTextureId, GL_TEXTURE_CUBE_MAP, "Resources/DaylightBoxUV.png", GL_RGBA);
	// TODO: Add explicit cubemap support in Renderer

	prepareBox();
	prepareReflectiveBox();
	prepareSkybox();

	camera.pos = glm::vec3(0.0f, 0.0f, 0.0f);

	renderer.prepareForRun();
}

void Skybox::prepareBox()
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

	renderer.generateVertexArray(boxVertexArrayId, vertexBuffer, 0, attribs);

	uint32_t texture0;
	renderer.generateTexture(texture0, "Resources/TutorialDiffuseMap.png", GL_RGBA);
	uint32_t texture1;
	renderer.generateTexture(texture1, "Resources/TutorialSpecularMap.png", GL_RGBA);
	boxTextureIds.clear();
	boxTextureIds.push_back(texture0);
	boxTextureIds.push_back(texture1);

	renderer.generateProgram(boxProgramId, "Shaders/LightingVS.glsl", "Shaders/SimpleLightingFS.glsl");
	renderer.setUniform3f(boxProgramId, "viewPos", camera.pos);

	renderer.setUniform1i(boxProgramId, "material.diffuse", 0);
	renderer.setUniform1i(boxProgramId, "material.specular", 1);
	renderer.setUniform1f(boxProgramId, "material.shininess", 32.0f);

	glm::vec3 simpleLightPos = glm::vec3(-1.2f, 1.0f, -2.0f);
	renderer.setUniform3f(boxProgramId, "simpleLight.position", simpleLightPos);
	renderer.setUniform3f(boxProgramId, "simpleLight.ambient", glm::vec3(0.4f, 0.4f, 0.4f));
	renderer.setUniform3f(boxProgramId, "simpleLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	renderer.setUniform3f(boxProgramId, "simpleLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
}

void Skybox::prepareReflectiveBox()
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

	renderer.generateVertexArray(reflectiveBoxVertexArrayId, vertexBuffer, 0, attribs);

	renderer.generateProgram(reflectiveBoxProgramId, "Shaders/ReflectionVS.glsl", "Shaders/ReflectionFS.glsl");
	renderer.setUniform1i(reflectiveBoxProgramId, "skybox", 0);
}

void Skybox::prepareSkybox()
{
	glGenTextures(1, &skyboxTextureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId);

	int width, height, channelCount;
	void* data;
	// Must be in +X, -X, +Y, -Y, +Z, -Z order
	/*std::vector<std::string> textureFaces =
	{
		"DaylightBoxRight.bmp", "DaylightBoxLeft.bmp", "DaylightBoxTop.bmp", "DaylightBoxBottom.bmp", "DaylightBoxFront.bmp", "DaylightBoxBack.bmp"
	};*/

	std::vector<std::string> textureFaces =
	{
		"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
	};
	stbi_set_flip_vertically_on_load(false);
	for (uint32_t i = 0; i < textureFaces.size(); i++)
	{
		data = stbi_load(("Resources/TutorialSkybox/" + textureFaces[i]).c_str(), &width, &height, &channelCount, 0);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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

void Skybox::run()
{
	renderer.prepareForRender();
	
	// -------- Box --------
	{
		renderer.prepareForDraw(boxProgramId, boxTextureIds, boxVertexArrayId);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		renderer.updateModelMatrix(model);
		renderer.setUniformMatrix4fv(boxProgramId, "normalMatrix", glm::transpose(glm::inverse(model)));
		camera.updateView();
		renderer.updateViewMatrix(camera.view);
		renderer.applyMvp(boxProgramId, "model", "view", "projection");
		renderer.setUniform3f(boxProgramId, "viewPos", camera.pos);
		renderer.draw(36);

		renderer.unprepareForDraw(boxProgramId, boxTextureIds);
	}

	// -------- Reflective Box --------
	{
		renderer.prepareForDraw(reflectiveBoxProgramId, { skyboxTextureId }, reflectiveBoxVertexArrayId);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		renderer.updateModelMatrix(model);
		camera.updateView();
		renderer.updateViewMatrix(camera.view);
		renderer.applyMvp(reflectiveBoxProgramId, "model", "view", "projection");
		renderer.setUniform1i(reflectiveBoxProgramId, "reflective", 1);
		renderer.setUniform3f(reflectiveBoxProgramId, "cameraPos", camera.pos);
		renderer.draw(36);

		

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
		renderer.updateModelMatrix(model);
		renderer.applyMvp(reflectiveBoxProgramId, "model", "view", "projection");
		renderer.setUniform1i(reflectiveBoxProgramId, "reflective", 0);
		renderer.draw(36);

		renderer.unprepareForDraw(reflectiveBoxProgramId, { skyboxTextureId });
	}
	
	
	// -------- Skybox --------
	{
		renderer.prepareForDraw(skyboxProgramId, { skyboxTextureId }, skyboxVertexArrayId);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glm::mat4 model = glm::mat4(1.0f);
		renderer.updateModelMatrix(model);
		camera.updateView();
		camera.view = glm::mat4(glm::mat3(camera.view));
		renderer.updateViewMatrix(camera.view);
		renderer.applyMvp(skyboxProgramId, "", "view", "projection");
		renderer.draw(36);

		renderer.unprepareForDraw(skyboxProgramId, { skyboxTextureId });
	}

	

	renderer.calculateFps();
	renderer.updateGLFW();
}

bool Skybox::shouldEnd()
{ 
	return renderer.getWindowShouldClose();
}
void Skybox::terminate()
{
	renderer.terminateGLFW();
}