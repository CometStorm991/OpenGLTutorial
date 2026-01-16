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

	glGenTextures(1, &skyboxTextureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId);

	int width, height, channelCount;
	void* data;
	// Must be in +X, -X, +Y, -Y, +Z, -Z order
	std::vector<std::string> textureFaces =
	{
		"DaylightBoxRight.bmp", "DaylightBoxLeft.bmp", "DaylightBoxTop.bmp", "DaylightBoxBottom.bmp", "DaylightBoxFront.bmp", "DaylightBoxBack.bmp"
	};
	for (uint32_t i = 0; i < textureFaces.size(); i++)
	{
		data = stbi_load(("Resources/DaylightBoxFaces/" + textureFaces[i]).c_str(), &width, &height, &channelCount, 0);
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

	uint32_t vertexBuffer;
	renderer.generateVertexBuffer(vertexBuffer, skyboxVertices);

	AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
	std::vector<AttributeLayout> attribs = { posAttrib };

	renderer.generateVertexArray(vertexArray, vertexBuffer, 0, attribs);

	renderer.addTexture(skyboxTextureId, GL_TEXTURE_CUBE_MAP);
	
	renderer.generateProgram(skyboxProgramId, "Shaders/SkyboxVS.glsl", "Shaders/SkyboxFS.glsl");

	uint32_t texture0;
	renderer.generateTexture(texture0, "Resources/TutorialDiffuseMap.png", GL_RGBA);
	uint32_t texture1;
	renderer.generateTexture(texture0, "Resources/TutorialSpecularMap.png", GL_RGBA);
	textureIds.clear();
	textureIds.push_back(texture0);
	textureIds.push_back(texture1);

	camera.pos = glm::vec3(0.0f, 0.0f, 0.0f);

	renderer.prepareForRun();
}

void Skybox::run()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderer.prepareForRender();
	renderer.prepareForDraw(skyboxProgramId, { skyboxTextureId }, vertexArray);
	
	glm::mat4 model = glm::mat4(1.0f);
	renderer.updateModelMatrix(model);
	renderer.applyMvp(skyboxProgramId, "", "view", "projection");
	renderer.draw(36);

	renderer.unprepareForDraw(skyboxProgramId, { skyboxTextureId });

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