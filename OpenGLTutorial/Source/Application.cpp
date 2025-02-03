#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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

    uint32_t program = glCreateProgram();

    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSource;
    std::ifstream vertexShaderFile("Shaders/VertexShader.glsl");
    while (vertexShaderFile) {
        std::string input;
        getline(vertexShaderFile, input);
        vertexShaderSource += input + "\n";
    }
    const char* vertexSourceCStr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSourceCStr, nullptr);
    glCompileShader(vertexShader);

    int32_t vertexResult;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexResult);
    if (vertexResult == GL_FALSE) {
        int32_t length;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(vertexShader, length, &length, message);
        std::cout << "Failed to compile vertex shader." << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(vertexShader);
    }

    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSource;
    std::ifstream fragmentShaderFile("Shaders/FragmentShader.glsl");
    while (fragmentShaderFile) {
        std::string input;
        getline(fragmentShaderFile, input);
        fragmentShaderSource += input + "\n";
    }
    const char* fragmentSourceCStr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentSourceCStr, nullptr);
    glCompileShader(fragmentShader);

    int32_t fragmentResult;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentResult);
    if (fragmentResult == GL_FALSE) {
        int32_t length;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(fragmentShader, length, &length, message);
        std::cout << "Failed to compile fragment shader." << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(fragmentShader);
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glValidateProgram(program);
    glUseProgram(program);

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
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