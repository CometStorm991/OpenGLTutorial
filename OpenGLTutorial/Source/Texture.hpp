#pragma once

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <STB/stb_image.h>

class Texture
{
private:
	std::string imagePath;
	GLenum textureUnit;

	uint32_t id;

	bool loaded = false;
public:
	Texture(const std::string& imagePath, GLenum textureUnit);

	void load();
	
	void use();
	void unuse();

	uint32_t getId();
	bool getLoaded();
};