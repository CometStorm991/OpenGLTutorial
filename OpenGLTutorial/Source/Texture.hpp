#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <STB/stb_image.h>

#include "TextureParameter.hpp"
#include "TextureSetup.hpp"

class Texture
{
public:
	Texture(GLenum target);
	// For 2D textures that are loaded using an image path
	static Texture ResourceTexture2D(const std::string& imagePath, bool flip, GLenum target, uint32_t textureUnit);
	// For cubemap textures that are loaded using an image path
	static Texture ResourceTextureCubemap(const std::vector<std::string>& imagePaths, bool flip, GLenum target, uint32_t textureUnit);
	// For textures that are used as attachments for framebuffers
	static Texture FramebufferTexture(uint32_t width, uint32_t height);
	// For textures that are managed externally
	static Texture ExternalTexture(uint32_t id, GLenum target);

	void setup(const TextureSetup& textureSetup);
	
	void use();

	std::string imagePath;
	GLenum pixelFormat;

	uint32_t id;
	GLenum target;
	uint32_t textureUnit;
	uint32_t width;
	uint32_t height;
	std::vector<void*> data;

	bool isSetup = false;
};