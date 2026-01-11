#pragma once

#include <GL/glew.h>
#include <STB/stb_image.h>

class TextureParameter
{
private:
	GLenum parameter;
	// Careful, assumes every parameter we're using can be set by glTexParameteri, excluding glTexParameterf
	GLint argument;
public:
	TextureParameter(GLenum parameter, GLint argument);

	GLenum getParameter();
	GLint getArgument();
};