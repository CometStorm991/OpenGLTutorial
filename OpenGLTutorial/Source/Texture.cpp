#include "Texture.hpp"

Texture::Texture(const std::string& imagePath, GLenum pixelFormat)
	: imagePath(imagePath), pixelFormat(pixelFormat), id(0), isSetup(false)
{
    int width, height, channelCount;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channelCount, 0);
    if (!data)
    {
        std::cout << "[Error] Failed to load texture" << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
    }

    if (width <= 0 || height <= 0) {
        std::cout << "[Error] Width or height is equal to or less than 0" << std::endl;
        std::cout << "Width is " << width << "Height is " << height << std::endl;
    }

    this->width = width;
    this->height = height;
    this->data = data;
}

// For textures that are used as attachments for framebuffers
Texture::Texture(uint32_t width, uint32_t height)
    : imagePath(""), pixelFormat(GL_RGB), id(0), width(width), height(height), data(nullptr), isSetup(false)
{
    
}

void Texture::setup(const std::vector<TextureParameter>& textureParameters)
{
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    for (TextureParameter textureParameter : textureParameters)
    {
        glTexParameteri(GL_TEXTURE_2D, textureParameter.getParameter(), textureParameter.getArgument());
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    isSetup = true;
}

void Texture::use(GLenum textureUnit)
{
    if (!isSetup)
    {
        std::cout << "[Error] Texture was not loaded" << std::endl;
    }

    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unuse(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

uint32_t Texture::getId()
{
    return id;
}

bool Texture::getIsSetup()
{
    return isSetup;
}