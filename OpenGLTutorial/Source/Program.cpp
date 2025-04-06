#include "Program.hpp"

Program::Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	:
	vertexShaderPath(vertexShaderPath),
	fragmentShaderPath(fragmentShaderPath),
	vertexShader(Shader(GL_VERTEX_SHADER, "")),
	fragmentShader(Shader(GL_FRAGMENT_SHADER, ""))
	/*vertexShader(vertexShader), fragmentShader(fragmentShader), id(0), programLoaded(false), beingUsed(false)*/
{
	
}

void Program::load()
{
	id = glCreateProgram();

	/*bool allShadersLoaded = true;
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
	}*/

	vertexShader = Shader(GL_VERTEX_SHADER, vertexShaderPath);
	fragmentShader = Shader(GL_FRAGMENT_SHADER, fragmentShaderPath);
	vertexShader.load();
	fragmentShader.load();

	/*uint32_t vertexShaderId = vertexShader->getId();
	uint32_t fragmentShaderId = fragmentShader->getId();*/

	uint32_t vertexShaderId = vertexShader.getId();
	uint32_t fragmentShaderId = fragmentShader.getId();

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
	beingUsed = true;
}

void Program::unuse()
{
	glUseProgram(0);
	beingUsed = false;
}

bool Program::getBeingUsed()
{
	return beingUsed;
}

uint32_t Program::getId() const
{
	return id;
}