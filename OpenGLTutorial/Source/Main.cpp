#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <random>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <STB/stb_image.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Program.hpp"
#include "Shader.hpp"
#include "Application.hpp"

template <class ClockType>
uint64_t getMillisecondsSinceTimePoint(std::chrono::time_point<ClockType> start)
{
    std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
    std::chrono::duration duration = current - start;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

uint32_t loadTexture(const std::string& imagePath, GLenum textureUnit)
{
    int width, height, channelCount;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channelCount, 0);
    if (!data)
    {
        std::cout << "[Error] Failed to load texture" << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
    }

    uint32_t textureId;
    glGenTextures(1, &textureId);
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
}

void testGLM()
{
    glm::vec4 vector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
    vector = trans * vector;
    std::cout << "X: " << vector.x << " Y: " << vector.y << " Z: " << vector.z << std::endl;
}



int main(void)
{
    Application app = Application();

    GLFWwindow* window = app.initWindow();
    app.initOpenGL();

    std::vector<float> cubeVertices;
    app.generateCube(cubeVertices);

    uint32_t vertexBuffer;
    app.generateVertexBuffer(vertexBuffer, cubeVertices);

    /*uint32_t indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), elements, GL_STATIC_DRAW);*/

    uint32_t texture0 = loadTexture("Resources/NeutronStar.jpg", GL_TEXTURE0);
    uint32_t texture1 = loadTexture("Resources/ArchLinux.jpeg", GL_TEXTURE1);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0); // Binds vertex buffer in GL_ARRAY_BUFFER to VAO

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float))); // Same vertex buffer is used for texture coords

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer); // Binds element buffer in GL_ELEMENT_ARRAY_BUFFER to VAO

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    
    std::shared_ptr<Shader> vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    std::shared_ptr<Shader> fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");
    vertexShader->load();
    fragmentShader->load();
    Program program = Program(vertexShader, fragmentShader);
    program.load();
    program.unuse();

    std::random_device randomDevice = std::random_device();
    std::default_random_engine randomEngine = std::default_random_engine(randomDevice());

    unsigned int cubeCount = 1000;
    // Desktop can handle 1200 cubes at 144 fps
    // Laptop can handle 1000 cubes at about 144 fps

    std::vector<glm::vec3> cubePositions = std::vector<glm::vec3>();
    cubePositions.reserve(cubeCount);
    std::uniform_real_distribution<float> posDistrib = std::uniform_real_distribution<float>(-10.0f, 10.0f);
    for (unsigned int i = 0; i < cubeCount; i++)
    {
        float x = posDistrib(randomEngine);
        float y = posDistrib(randomEngine);
        float z = posDistrib(randomEngine);
        cubePositions.push_back(glm::vec3(x, y, z));
    }

    std::vector<glm::vec3> cubeRotationSpeeds = std::vector<glm::vec3>();
    cubeRotationSpeeds.reserve(cubeCount);
    std::uniform_real_distribution<float> speedDistrib = std::uniform_real_distribution<float>(0.0f, 5.0f);
    for (unsigned int i = 0; i < cubeCount; i++)
    {
        float x = speedDistrib(randomEngine);
        float y = speedDistrib(randomEngine);
        float z = speedDistrib(randomEngine);
        cubeRotationSpeeds.push_back(glm::vec3(x, y, z));
    }

    glm::mat4 model = glm::mat4(1.0f);
    
    glm::mat4 view = glm::mat4(1.0f);

    float cameraSpeed = 1.0f;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -30.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    

    

    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));


    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    glm::mat4 mvp = glm::mat4(1.0f);

    program.use();
    glUniform1i(glGetUniformLocation(program.getId(), "inputTexture0"), 0);
    glUniform1i(glGetUniformLocation(program.getId(), "inputTexture1"), 1);

    uint32_t mvpLoc = glGetUniformLocation(program.getId(), "mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    program.unuse();

    glEnable(GL_DEPTH_TEST);
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point lastSecond = std::chrono::steady_clock::now();
    uint32_t leftOverMillis = 0;
    uint32_t fps = 0;

    uint32_t previousMillis = 0;

    /* Loop until the user closes the window */
    
    while (!glfwWindowShouldClose(window))
    {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.use();

        uint32_t milliseconds = getMillisecondsSinceTimePoint(start);
        float cameraX = std::sin(milliseconds / 1000.0f * 2.0f * std::numbers::pi_v<float> * 0.2f) * 30.0f;
        float cameraZ = std::cos(milliseconds / 1000.0f * 2.0f * std::numbers::pi_v<float> * 0.2f) * 30.0f;



        /*view = glm::lookAt(
            glm::vec3(cameraX, 0.0f, cameraZ),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );*/

        float yaw = app.getYaw();
        float pitch = app.getPitch();
        glm::vec3 cameraDirection;
        cameraDirection.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        cameraDirection.y = std::sin(glm::radians(pitch));
        cameraDirection.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

        cameraFront = glm::normalize(cameraDirection);
        cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

        float deltaTime = (milliseconds - previousMillis) / 1000.0f;
        cameraSpeed = deltaTime * 10.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos += cameraSpeed * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos -= cameraSpeed * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos += cameraSpeed * cameraRight;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos -= cameraSpeed * cameraRight;
        }


        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glBindVertexArray(vao);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        for (unsigned int i = 0; i < cubeCount; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions.at(i));

            glm::vec3 rotationSpeed = cubeRotationSpeeds.at(i);
            model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.z, glm::vec3(0.0f, 0.0f, 1.0f));

            mvp = projection * view * model;
            uint32_t mvpLoc = glGetUniformLocation(program.getId(), "mvp");
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        program.unuse();
        
        fps++;
        uint32_t currentSecMillis = getMillisecondsSinceTimePoint(lastSecond);
        if (currentSecMillis + leftOverMillis > 1000)
        {
            leftOverMillis = currentSecMillis + leftOverMillis - 1000;
            lastSecond = std::chrono::steady_clock::now();
            std::cout << "FPS: " << fps << std::endl;
            fps = 0;
        }

        previousMillis = milliseconds;

        
        glfwSwapBuffers(window);

        
        glfwPollEvents();
    }
    

    glfwTerminate();
    return 0;
}