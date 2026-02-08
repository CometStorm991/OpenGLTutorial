#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include <GL/glew.h>

class Shader
{
private:
	GLenum shaderType;
	std::string filePath;

	uint32_t id;

	bool loaded;
public:
	Shader(GLenum shaderType, const std::string& filePath);

	void load();
	void load(const std::string& header);

	bool getShaderLoaded() const;
	uint32_t getId() const;

	static std::string getShaderTypeStr(GLenum shaderType);
};
