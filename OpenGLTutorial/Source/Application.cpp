#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include "Program.hpp"
#include "Shader.hpp"

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

    //std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[16] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f
    };

    uint32_t elements[6] = {
        0, 1, 2,
        2, 3, 1
    };

    int width, height, channelCount;
    unsigned char* data = stbi_load("Resources/NeutronStar.jpg", &width, &height, &channelCount, 0);
    if (!data)
    {
        std::cout << "[Error] Failed to load texture" << std::endl;
    }

    uint32_t vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), positions, GL_STATIC_DRAW);

    uint32_t indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), elements, GL_STATIC_DRAW);

    uint32_t texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)0); // Binds vertex buffer in GL_ARRAY_BUFFER to VAO

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float))); // Same vertex buffer is used for texture coords

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer); // Binds element buffer in GL_ELEMENT_ARRAY_BUFFER to VAO

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::shared_ptr<Shader> vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    std::shared_ptr<Shader> fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");
    vertexShader->load();
    fragmentShader->load();
    Program program = Program(vertexShader, fragmentShader);
    program.load();
    program.unuse();

    
    // Have to keep element array buffer bound for drawing

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

        //glDrawArrays(GL_TRIANGLES, 0, 3);
        program.use();
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}