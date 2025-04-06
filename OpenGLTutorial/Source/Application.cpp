#include "Application.hpp"

void Application::init()
{
    renderer = Renderer();
    renderer.init();
}

void Application::prepare()
{
    prepareLighting();
    prepareForRun();
}

void Application::addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao)
{
    std::vector<float> cubeVertices;
    Cube::generatePT(cubeVertices);

    uint32_t vertexBuffer;
    renderer.generateVertexBuffer(vertexBuffer, cubeVertices);

    AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
    AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

    std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
    attribs.push_back(posAttrib);
    attribs.push_back(texAttrib);

    renderer.generateVertexArray(vao, vertexBuffer, attribs);

    uint32_t texture0;
    renderer.generateTexture(texture0, "Resources/NeutronStar.jpg", GL_TEXTURE0);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/ArchLinux.jpeg", GL_TEXTURE1);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);
}

void Application::prepareGettingStarted()
{
    addCubeVertices(textureIds, vaoId);

    renderer.generateProgram(programId, "Shaders/GettingStartedVS.glsl", "Shaders/GettingStartedFS.glsl");

    renderer.setUniform1i(programId, "inputTexture0", 0);
    renderer.setUniform1i(programId, "inputTexture1", 1);

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

    renderer.setCameraPos(glm::vec3(0.0f, 0.0f, -30.0f));
}

void Application::prepareLighting()
{
    std::vector<float> cubeVertices;
    Cube::generatePNT(cubeVertices);

    uint32_t vertexBuffer;
    renderer.generateVertexBuffer(vertexBuffer, cubeVertices);

    AttributeLayout posAttrib = AttributeLayout(3, GL_FLOAT);
    AttributeLayout normAttrib = AttributeLayout(3, GL_FLOAT);
    AttributeLayout texAttrib = AttributeLayout(2, GL_FLOAT);

    std::vector<AttributeLayout> attribs = std::vector<AttributeLayout>();
    attribs.push_back(posAttrib);
    attribs.push_back(normAttrib);
    attribs.push_back(texAttrib);

    renderer.generateVertexArray(vaoId, vertexBuffer, attribs);

    uint32_t texture0;
    renderer.generateTexture(texture0, "Resources/NeutronStar.jpg", GL_TEXTURE0);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/ArchLinux.jpeg", GL_TEXTURE1);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);

    renderer.generateProgram(programId, "Shaders/LightingVS.glsl", "Shaders/LightingRegularFS.glsl");
    renderer.generateProgram(lightProgramId, "Shaders/LightingVS.glsl", "Shaders/LightingLightFS.glsl");

    lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

    renderer.setUniform3f(programId, "objectColor", glm::vec3(0.5f, 0.0f, 1.0f));
    renderer.setUniform3f(programId, "lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    renderer.setUniform3f(programId, "lightPos", lightPos);

    renderer.setCameraPos(glm::vec3(0.0f, 0.0f, -10.0f));
}

void Application::prepareForRun()
{
    renderer.prepareForRun();
}

void Application::run()
{
    runLighting();
}

void Application::runGettingStarted()
{
    renderer.prepareForDraw(programId, textureIds, vaoId);

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
        renderer.applyMvp(programId, "model", "view", "projection");
        renderer.draw(36);
    }

    renderer.unprepareForDraw(programId, textureIds);
    renderer.calculateFps();
    renderer.updateGLFW();
}

void Application::runLighting()
{
    renderer.prepareForRender();
    renderer.calculateCameraTransform();

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);
    
    renderer.updateModelMatrix(glm::mat4(1.0f));
    renderer.applyMvp(programId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(programId, textureIds);

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(lightProgramId, textureIds, vaoId);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    renderer.updateModelMatrix(model);
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);

    // -------------------------------------------------------------------------

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