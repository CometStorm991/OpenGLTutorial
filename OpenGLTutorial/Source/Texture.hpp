#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <STB/stb_image.h>

#include "TextureParameter.hpp"

class Texture
{
private:
	std::string imagePath;
	GLenum pixelFormat;

	uint32_t id;
	uint32_t width;
	uint32_t height;
	void* data;

	bool isSetup = false;
public:
	Texture(const std::string& imagePath, GLenum pixelFormat);
	Texture(uint32_t width, uint32_t height);

	void setup(const std::vector<TextureParameter>& textureParameters);
	
	void use(GLenum textureUnit);
	void unuse(GLenum textureUnit);

	uint32_t getId();
	bool getIsSetup();
};