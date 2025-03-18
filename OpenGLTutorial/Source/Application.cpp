#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Program.hpp"
#include "Shader.hpp"

uint32_t compileShader(GLenum shaderType, const std::string& shaderFilePath)
{
    uint32_t shader = glCreateShader(shaderType);

    // Reading shader code from file
    std::string shaderSource;
    std::ifstream shaderFile(shaderFilePath);
    while (shaderFile)
    {
        std::string input;
        getline(shaderFile, input);
        shaderSource += input + "\n";
    }

    // Loading shader
    const char* sourceCStr = shaderSource.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // Validating shader
    int32_t result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int32_t length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);
        std::cout << "Failed to compile vertex shader." << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

template <class ClockType>
uint64_t getMillisecondsSinceTimePoint(std::chrono::time_point<ClockType> start)
{
    std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
    std::chrono::duration duration = current - start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 360, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Glew failed." << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[6] = {
        -0.5f, -0.5f,
         0.0f,  0.25f * std::sqrt(3.0f),
         0.5f, -0.5f,
    };

    uint32_t vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (const void*)0);

    /*uint32_t program = glCreateProgram();

    uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glValidateProgram(program);
    glUseProgram(program);

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);*/
    
    std::shared_ptr<Shader> vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    std::shared_ptr<Shader> fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");
    vertexShader->load();
    fragmentShader->load();
    Program program = Program(vertexShader, fragmentShader);
    program.load();

    //Program program = Program();

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        uint32_t milliseconds = getMillisecondsSinceTimePoint(start);
        float redColor = std::sin(milliseconds / 1000.0f * 2.0f * std::numbers::pi_v<float>) / 2.0f + 0.5f;

        int redColorLocation = glGetUniformLocation(program.getId(), "redColor");
        glUniform1f(redColorLocation, redColor);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}