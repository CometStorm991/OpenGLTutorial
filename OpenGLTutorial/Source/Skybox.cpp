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
		data = stbi_load(textureFaces[i].c_str(), &width, &height, &channelCount, 0);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::run() {}
bool Skybox::shouldEnd() { return false; }
void Skybox::terminate() {}