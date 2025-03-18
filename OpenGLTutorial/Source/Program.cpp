#include "Program.hpp"

Program::Program(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader)
	: vertexShader(vertexShader), fragmentShader(fragmentShader), id(0), programLoaded(false)
{
}

void Program::load()
{
	id = glCreateProgram();
	if (!vertexShader->getShaderLoaded() || !fragmentShader->getShaderLoaded())
	{
		std::cout << "[Error] Not all shaders were loaded" << std::endl;
		return;
	}

	uint32_t vertexShaderId = vertexShader->getId();
	uint32_t fragmentShaderId = fragmentShader->getId();

	glAttachShader(id, vertexShaderId);
	glAttachShader(id, fragmentShaderId);
	glLinkProgram(id);
	glValidateProgram(id);
	glUseProgram(id);

	glDetachShader(id, vertexShaderId);
	glDetachShader(id, fragmentShaderId);
	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	programLoaded = true;
}

uint32_t Program::getId() const
{
	return id;
}