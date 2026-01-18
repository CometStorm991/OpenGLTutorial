#include "Texture.hpp"

Texture::Texture(GLenum target)
    : imagePath(""), pixelFormat(GL_RGB), id(0), width(0), height(0), data({}), isSetup(false), target(target), textureUnit(0)
{

}

Texture Texture::ResourceTexture2D(const std::string& imagePath, bool flip, GLenum target, uint32_t textureUnit)
{
    if (imagePath.empty())
    {
        std::cout << "[Error] Image path is empty" << std::endl;
    }

    Texture texture = Texture(target);
    texture.textureType = TextureType::RESOURCE_TEXTURE_2D;
    texture.textureUnit = textureUnit;
    
    int width, height, channelCount;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channelCount, 0);
    if (!data)
    {
        std::cout << "[Error] Failed to load texture" << imagePath << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
    }

    if (width <= 0 || height <= 0) {
        std::cout << "[Error] Width or height is equal to or less than 0" << std::endl;
        std::cout << "Width is " << width << "Height is " << height << std::endl;
    }
    if (channelCount < 3 || channelCount > 4)
    {
        std::cout << "[Error]: Channel count is " << channelCount << std::endl;
    }

    texture.width = width;
    texture.height = height;
    texture.data = { data };
    switch (channelCount)
    {
    case 3:
        texture.pixelFormat = GL_RGB;
        break;
    case 4:
    default:
        texture.pixelFormat = GL_RGBA;
        break;
    }

    return texture;
}

Texture Texture::ResourceTextureCubemap(const std::vector<std::string>& imagePaths, bool flip, GLenum target, uint32_t textureUnit)
{
    Texture texture = Texture(target);
    texture.textureType = TextureType::RESOURCE_TEXTURE_CUBEMAP;
    texture.textureUnit = textureUnit;

    int width, height, channelCount;
    stbi_set_flip_vertically_on_load(flip);
    for (uint32_t i = 0; i < imagePaths.size(); i++)
    {
        void* currentData = stbi_load((imagePaths[i]).c_str(), &width, &height, &channelCount, 0);
        texture.data.push_back(currentData);
        /*glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);*/
        
    }
    if (width <= 0 || height <= 0) {
        std::cout << "[Error] Width or height is equal to or less than 0" << std::endl;
        std::cout << "Width is " << width << "Height is " << height << std::endl;
    }
    if (channelCount < 3 || channelCount > 4)
    {
        std::cout << "[Error]: Channel count is " << channelCount << std::endl;
    }

    texture.width = width;
    texture.height = height;
    switch (channelCount)
    {
    case 3:
        texture.pixelFormat = GL_RGB;
        break;
    case 4:
    default:
        texture.pixelFormat = GL_RGBA;
        break;
    }

    return texture;
}

Texture Texture::FramebufferTexture(uint32_t width, uint32_t height)
{
    Texture texture = Texture(GL_TEXTURE_2D);
    texture.textureType = TextureType::FRAMEBUFFER_TEXTURE;
    texture.width = width;
    texture.height = height;

    return texture;
}

Texture Texture::ExternalTexture(uint32_t id, GLenum target)
{
    Texture texture = Texture(target);
    texture.textureType = TextureType::EXTERNAL_TEXTURE;
    texture.id = id;
    texture.isSetup = true;

    return texture;
}

void Texture::setup(const TextureSetup& textureSetup)
{
    if (textureType == TextureType::EXTERNAL_TEXTURE)
    {
        isSetup = true;
        return;
    }

    if (isSetup)
    {
        std::cout << "[Error]: Texture " << imagePath << " is already setup" << std::endl;
    }

    glCreateTextures(target, 1, &id);

    glTextureStorage2D(id, 1, GL_RGB8, width, height);
    switch (textureType)
    {
    case TextureType::RESOURCE_TEXTURE_CUBEMAP:
        for (uint32_t i = 0; i < data.size(); i++)
        {
            glTextureSubImage3D(id, 0, 0, 0, i, width, height, 1, pixelFormat, GL_UNSIGNED_BYTE, data[i]);
            stbi_image_free(data[i]);
        }
        break;
    case TextureType::RESOURCE_TEXTURE_2D:
        if (data[0] != nullptr)
        {
            glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE, data[0]);
            stbi_image_free(data[0]);
        }
        break;
    }

    for (TextureParameter textureParameter : textureSetup.textureParameters)
    {
        glTextureParameteri(id, textureParameter.getParameter(), textureParameter.getArgument());
    }

    if (textureSetup.mipmap)
    {
        glGenerateTextureMipmap(id);
    }

    isSetup = true;
}

void Texture::use()
{
    if (!isSetup)
    {
        std::cout << "[Error] Texture was not setup" << std::endl;
    }

    glBindTextureUnit(textureUnit, id);
}