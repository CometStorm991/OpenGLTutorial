#include "GettingStarted.hpp"

GettingStarted::GettingStarted()
    : renderer(Renderer(camera))
{
    
}

void GettingStarted::init()
{
    renderer.init();
}

void GettingStarted::prepare()
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

    camera.pos = glm::vec3(0.0f, 0.0f, -30.0f);

    renderer.prepareForRun();
}

void GettingStarted::addCubeVertices(std::vector<uint32_t>& textureIds, uint32_t& vao)
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

    renderer.generateVertexArray(vao, vertexBuffer, 0, attribs);

    uint32_t texture0;
    renderer.generateResourceTexture2D(texture0, "Resources/NeutronStar.jpg", true, GL_TEXTURE_2D, 0);
    uint32_t texture1;
    renderer.generateResourceTexture2D(texture1, "Resources/ArchLinux.jpeg", true, GL_TEXTURE_2D, 0);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);
}

void GettingStarted::run()
{
    renderer.prepareForRender();

    renderer.prepareForDraw(programId, textureIds, vaoId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();

    for (unsigned int i = 0; i < cubeCount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions.at(i));

        glm::vec3 rotationSpeed = cubeRotationSpeeds.at(i);
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.z, glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.updateModelMatrix(model);
        camera.updateView();
        renderer.updateViewMatrix(camera.view);
        renderer.applyMvp(programId, "model", "view", "projection");
        renderer.draw(36);
    }

    renderer.unprepareForDraw(programId, textureIds);
    renderer.calculateFps();
    renderer.updateGLFW();
}

bool GettingStarted::shouldEnd()
{
    return renderer.getWindowShouldClose();
}

void GettingStarted::terminate()
{
    renderer.terminateGLFW();
}