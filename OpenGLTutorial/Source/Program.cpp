#include "Program.hpp"

Program::Program(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	shaderPaths.push_back(vertexShaderPath);
	shaderPaths.push_back(fragmentShaderPath);

	shaders.push_back(Shader(GL_VERTEX_SHADER, vertexShaderPath));
	shaders.push_back(Shader(GL_FRAGMENT_SHADER, fragmentShaderPath));

	headers.push_back("");
	headers.push_back("");
}

Program::Program(const std::vector<ShaderInfo> shaderInfos)
{
	for (const ShaderInfo& shaderInfo : shaderInfos)
	{
		shaderPaths.push_back(shaderInfo.path);
		shaders.emplace_back(Shader(shaderInfo.type, shaderInfo.path));
		headers.push_back(shaderInfo.header);
	}
}

void Program::load()
{
	id = glCreateProgram();

	for (int i = 0; i < shaders.size(); i++)
	{
		Shader& shader = shaders[i];

		shader.load(headers[i]);
		glAttachShader(id, shader.getId());
	}
	glLinkProgram(id);
	glValidateProgram(id);

	for (const Shader& shader : shaders)
	{
		glDetachShader(id, shader.getId());
		glDeleteShader(shader.getId());
	}

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