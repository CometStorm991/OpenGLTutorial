#pragma once

#include <GL/glew.h>

#include "Shader.hpp"

class Program
{
private:
	std::shared_ptr<Shader> vertexShader;
	std::shared_ptr<Shader> fragmentShader;

	uint32_t id;

	bool programLoaded;
	bool beingUsed;
public:
	Program(std::shared_ptr<Shader> vertexShader, std::shared_ptr<Shader> fragmentShader);

	void load();

	void use();
	void unuse();

	bool getBeingUsed();

	uint32_t getId() const;
};