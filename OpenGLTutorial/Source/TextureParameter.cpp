#include "TextureParameter.hpp"

TextureParameter::TextureParameter(GLenum parameter, GLint argument)
{
	this->parameter = parameter;
	this->argument = argument;
}

GLenum TextureParameter::getParameter()
{
	return parameter;
}

GLint TextureParameter::getArgument()
{
	return argument;
}