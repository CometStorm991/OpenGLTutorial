#include "Application.hpp"

void Application::init()
{
    renderer = Renderer();
    renderer.init();
}

void Application::prepare()
{
    prepareRotatingCubes();
    prepareForRun();
}

void Application::addCubeVertices()
{
    std::vector<float> cubeVertices;
    renderer.generateCube(cubeVertices);

    uint32_t vertexBuffer;
    renderer.generateVertexBuffer(vertexBuffer, cubeVertices);

    AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
    AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

    std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
    attribs.push_back(posAttrib);
    attribs.push_back(texAttrib);
    uint32_t vao;

    renderer.generateVertexArray(vao, vertexBuffer, attribs);

    uint32_t texture0;
    renderer.generateTexture(texture0, "Resources/NeutronStar.jpg", GL_TEXTURE0);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/ArchLinux.jpeg", GL_TEXTURE1);

    renderer.generateShaders();

    renderer.setUniform1i("inputTexture0", 0);
    renderer.setUniform1i("inputTexture1", 1);
}

void Application::prepareRotatingCubes()
{
    addCubeVertices();

    std::random_device randomDevice = std::random_device();
    std::default_random_engine randomEngine = std::default_random_engine(randomDevice());

    cubePositions = std::vector<glm::vec3>();
    cubePositions.reserve(cubeCount);
    std::uniform_real_distribution<float> posDistrib = std::uniform_real_distribution<float>(-10.0f, 10.0f);
    for (unsigned int i = 0; i < cubeCount; i++)
    {
        float x = posDistrib(randomEngine);
        float y = posDistrib(randomEngine);
        float z = posDistrib(randomEngine);
        cubePositions.push_back(glm::vec3(x, y, z));
    }

    cubeRotationSpeeds = std::vector<glm::vec3>();
    cubeRotationSpeeds.reserve(cubeCount);
    std::uniform_real_distribution<float> speedDistrib = std::uniform_real_distribution<float>(0.0f, 5.0f);
    for (unsigned int i = 0; i < cubeCount; i++)
    {
        float x = speedDistrib(randomEngine);
        float y = speedDistrib(randomEngine);
        float z = speedDistrib(randomEngine);
        cubeRotationSpeeds.push_back(glm::vec3(x, y, z));
    }
}

void Application::prepareForRun()
{
    renderer.prepareForRun();
}

void Application::run()
{
    renderer.prepareForDraw();

    uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();

    renderer.calculateCameraTransform();
    for (unsigned int i = 0; i < cubeCount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions.at(i));

        glm::vec3 rotationSpeed = cubeRotationSpeeds.at(i);
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.z, glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.updateModelMatrix(model);
        renderer.draw(36);
    }

    renderer.unprepareForDraw();
    renderer.calculateFps();
    renderer.updateGLFW();
}

void Application::terminate()
{
    renderer.terminateGLFW();
}

bool Application::shouldEnd()
{
    return renderer.getWindowShouldClose();
}