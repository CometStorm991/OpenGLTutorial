#pragma once

#include <cmath>

#include <GL/glew.h>

class AttributeLayout {
private:
	uint32_t count;
	GLenum type;

public:
	AttributeLayout(uint32_t count, GLenum type);

	uint32_t getCount();
	GLenum getType();
};