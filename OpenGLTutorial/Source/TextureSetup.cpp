#include "TextureSetup.hpp"

TextureSetup::TextureSetup(bool mipmap, const std::vector<TextureParameter>& textureParameters) : TextureSetup(mipmap, GL_RGB8, textureParameters)
{
	
}

TextureSetup::TextureSetup(bool mipmap, GLenum internalFormat, const std::vector<TextureParameter>& textureParameters)
	: mipmap(mipmap), internalFormat(internalFormat), textureParameters(textureParameters)
{

}