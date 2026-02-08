#pragma once

#include <cmath>

#include <GL/glew.h>

class AttributeLayout {
public:
	AttributeLayout(uint32_t count, GLenum type);
	AttributeLayout(uint32_t count, GLenum type, uint32_t layoutIndex);

	uint32_t getCount();
	GLenum getType();

	uint32_t count;
	GLenum type;
	uint32_t layoutIndex;
};