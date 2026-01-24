#pragma once

#include <vector>

#include "TextureParameter.hpp"

class TextureSetup
{
public:
	bool mipmap;
	GLenum internalFormat;
	std::vector<TextureParameter> textureParameters;

	TextureSetup(bool mipmap, const std::vector<TextureParameter>& textureParameters);
	TextureSetup(bool mipmap, GLenum internalFormat, const std::vector<TextureParameter>& textureParameters);
};