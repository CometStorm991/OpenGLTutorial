#include "Shader.hpp"

Shader::Shader(GLenum shaderType, const std::string& filePath)
    : shaderType(shaderType), filePath(filePath), loaded(false), id(0)
{
}

void Shader::load()
{
    id = glCreateShader(shaderType);

    // Reading shader code from file
    std::string source;
    std::ifstream file(filePath);
    if (!file.good())
    {
        std::cout << "[Error] File " << filePath << " does not exist" << std::endl;
        return;
    }
    while (file)
    {
        std::string input;
        getline(file, input);
        source += input + "\n";
    }

    // Loading shader
    const char* sourceCStr = source.c_str();
    glShaderSource(id, 1, &sourceCStr, nullptr);
    glCompileShader(id);

    // Validating shader
    int32_t result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int32_t length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "[Error] Failed to compile " << getShaderTypeStr(shaderType) << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return;
    }

    loaded = true;
}

bool Shader::getShaderLoaded() const
{
    return loaded;
}

uint32_t Shader::getId() const
{
    return id;
}

std::string Shader::getShaderTypeStr(GLenum shaderType)
{
    switch (shaderType)
    {
    case GL_VERTEX_SHADER:
        return "vertex shader";
    case GL_FRAGMENT_SHADER:
        return "fragment shader";
    }

    std::cout << "Couldn't get string version of " << shaderType << std::endl;

    return "";
}