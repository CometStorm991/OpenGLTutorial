#include "Texture.hpp"

Texture::Texture(const std::string& imagePath, GLenum textureUnit)
	: imagePath(imagePath), textureUnit(textureUnit), loaded(false)
{

}

void Texture::load()
{
    int width, height, channelCount;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channelCount, 0);
    if (!data)
    {
        std::cout << "[Error] Failed to load texture" << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
    }

    glGenTextures(1, &id);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    loaded = true;
}

void Texture::use()
{
    if (!loaded)
    {
        std::cout << "[Error] Texture was not loaded" << std::endl;
    }

    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unuse()
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}