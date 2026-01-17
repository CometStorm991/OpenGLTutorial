#pragma once

#include <vector>

#include "TextureParameter.hpp"

class TextureSetup
{
public:
	bool mipmap;
	std::vector<TextureParameter> textureParameters;

	TextureSetup(bool mipmap, const std::vector<TextureParameter>& textureParameters);
};