#pragma once

#include <string>

#include <GL/glew.h>

struct ShaderInfo
{
	GLenum type;
	std::string path;
	std::string header = "";
};