#pragma once

#include <GL/glew.h>

#include "Shader.hpp"

class Program
{
private:
	std::string vertexShaderPath;
	std::string fragmentShaderPath;

	/*std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;*/

	Shader vertexShader;
	Shader fragmentShader;

	uint32_t id;

	bool programLoaded;
	bool beingUsed;
public:
	Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

	void load();

	void use();
	void unuse();

	bool getBeingUsed();

	uint32_t getId() const;
};