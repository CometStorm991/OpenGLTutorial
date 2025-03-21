#include "Program.hpp"

Program::Program(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader)
	: vertexShader(vertexShader), fragmentShader(fragmentShader), id(0), programLoaded(false)
{
}

void Program::load()
{
	id = glCreateProgram();

	bool allShadersLoaded = true;
	if (!vertexShader->getShaderLoaded())
	{
		std::cout << "[Error] Vertex shader was not loaded" << std::endl;
		allShadersLoaded = false;
	}

	if (!fragmentShader->getShaderLoaded())
	{
		std::cout << "[Error] Fragment shader was not loaded" << std::endl;
		allShadersLoaded = false;
	}
	if (!allShadersLoaded)
	{
		return;
	}

	uint32_t vertexShaderId = vertexShader->getId();
	uint32_t fragmentShaderId = fragmentShader->getId();

	glAttachShader(id, vertexShaderId);
	glAttachShader(id, fragmentShaderId);
	glLinkProgram(id);
	glValidateProgram(id);

	glDetachShader(id, vertexShaderId);
	glDetachShader(id, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	programLoaded = true;
}

void Program::use()
{
	glUseProgram(id);
}

void Program::unuse()
{
	glUseProgram(0);
}

uint32_t Program::getId() const
{
	return id;
}