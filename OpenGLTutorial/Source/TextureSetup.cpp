#include "TextureSetup.hpp"

TextureSetup::TextureSetup(bool mipmap, const std::vector<TextureParameter>& textureParameters)
{
	this->mipmap = mipmap;
	this->textureParameters = textureParameters;
}