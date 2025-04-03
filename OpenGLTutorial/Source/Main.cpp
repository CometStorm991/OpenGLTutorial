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
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

#include "AttributeLayout.hpp"
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

int main(void)
{
    Application app = Application();

    GLFWwindow* window = app.initWindow();
    app.initOpenGL();

    std::vector<float> cubeVertices;
    app.generateCube(cubeVertices);
    
    uint32_t vertexBuffer;
    app.generateVertexBuffer(vertexBuffer, cubeVertices);

    AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
    AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

    std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
    attribs.push_back(posAttrib);
    attribs.push_back(texAttrib);
    uint32_t vao;

    app.generateVertexArray(vao, vertexBuffer, attribs);

    uint32_t texture0;
    app.generateTexture(texture0, "Resources/NeutronStar.jpg", GL_TEXTURE0);
    uint32_t texture1;
    app.generateTexture(texture1, "Resources/ArchLinux.jpeg", GL_TEXTURE1);

    
    std::shared_ptr<Shader> vertexShader = std::make_shared<Shader>(GL_VERTEX_SHADER, "Shaders/VertexShader.glsl");
    std::shared_ptr<Shader> fragmentShader = std::make_shared<Shader>(GL_FRAGMENT_SHADER, "Shaders/FragmentShader.glsl");
    vertexShader->load();
    fragmentShader->load();
    Program program = Program(vertexShader, fragmentShader);
    program.load();
    program.unuse();

    std::random_device randomDevice = std::random_device();
    std::default_random_engine randomEngine = std::default_random_engine(randomDevice());

    const unsigned int cubeCount = 1000;
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