#pragma once

#include <vector>

#include <GL/glew.h>

#include "Shader.hpp"
#include "ShaderInfo.hpp"

class Program
{
private:
	std::vector<std::string> shaderPaths{};
	std::vector<Shader> shaders{};

	uint32_t id = 0;

	bool programLoaded = false;
	bool beingUsed = false;
public:
	Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Program(const std::vector<ShaderInfo> shaderInfos);

	void load();

	void use();
	void unuse();

	bool getBeingUsed();

	uint32_t getId() const;
};