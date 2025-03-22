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

// This code is from learnopengl.com
void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
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
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    /*float positions[16] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f
    };*/

    float cubeVertices[180] = {
        // Front face
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        // Back face
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        // Left face
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        // Right face
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        // Top face
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,

        // Bottom face
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f
    };

    uint32_t vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 180 * sizeof(float), cubeVertices, GL_STATIC_DRAW);

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

    unsigned int cubeCount = 100;

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
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -30.0f));

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    glm::mat4 mvp = projection * view * model;

    program.use();
    glUniform1i(glGetUniformLocation(program.getId(), "inputTexture0"), 0);
    glUniform1i(glGetUniformLocation(program.getId(), "inputTexture1"), 1);

    uint32_t mvpLoc = glGetUniformLocation(program.getId(), "mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    program.unuse();

    glEnable(GL_DEPTH_TEST);
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    /* Loop until the user closes the window */
    
    while (!glfwWindowShouldClose(window))
    {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.use();

        uint32_t milliseconds = getMillisecondsSinceTimePoint(start);
        float redColor = std::sin(milliseconds / 1000.0f * 2.0f * std::numbers::pi_v<float>) / 2.0f + 0.5f;

        int redColorLocation = glGetUniformLocation(program.getId(), "redColor");
        glUniform1f(redColorLocation, redColor);

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

        
        glfwSwapBuffers(window);

        
        glfwPollEvents();
    }
    

    glfwTerminate();
    return 0;
}