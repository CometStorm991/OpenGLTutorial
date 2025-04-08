#include "Application.hpp"

void Application::init()
{
    renderer = Renderer();
    renderer.init();
}

void Application::prepare()
{
    prepareMultipleLighting();
    prepareForRun();
}

void Application::run()
{
    runMultipleLighting();
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
    renderer.generateTexture(texture0, "Resources/NeutronStar.jpg", GL_RGB);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/ArchLinux.jpeg", GL_RGB);
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
    cubePositions.reserve(cubeCountGS);
    std::uniform_real_distribution<float> posDistrib = std::uniform_real_distribution<float>(-10.0f, 10.0f);
    for (unsigned int i = 0; i < cubeCountGS; i++)
    {
        float x = posDistrib(randomEngine);
        float y = posDistrib(randomEngine);
        float z = posDistrib(randomEngine);
        cubePositions.push_back(glm::vec3(x, y, z));
    }

    cubeRotationSpeeds = std::vector<glm::vec3>();
    cubeRotationSpeeds.reserve(cubeCountGS);
    std::uniform_real_distribution<float> speedDistrib = std::uniform_real_distribution<float>(0.0f, 5.0f);
    for (unsigned int i = 0; i < cubeCountGS; i++)
    {
        float x = speedDistrib(randomEngine);
        float y = speedDistrib(randomEngine);
        float z = speedDistrib(randomEngine);
        cubeRotationSpeeds.push_back(glm::vec3(x, y, z));
    }

    renderer.setCameraPos(glm::vec3(0.0f, 0.0f, -30.0f));
}

void Application::addLightingInfo()
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
    renderer.generateTexture(texture0, "Resources/TutorialDiffuseMap.png", GL_RGBA);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/TutorialSpecularMap.png", GL_RGBA);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);
}

void Application::prepareSimpleLighting()
{
    addLightingInfo();

    renderer.generateProgram(programId, "Shaders/LightingVS.glsl", "Shaders/SimpleLightingFS.glsl");
    renderer.generateProgram(lightProgramId, "Shaders/LightingVS.glsl", "Shaders/LightingLightFS.glsl");

    simpleLightPos = glm::vec3(1.2f, 1.0f, 2.0f);
    
    renderer.setUniform3f(programId, "viewPos", renderer.getCameraPos());

    renderer.setUniform1i(programId, "material.diffuse", 0);
    renderer.setUniform1i(programId, "material.specular", 1);
    renderer.setUniform1f(programId, "material.shininess", 32.0f);

    renderer.setUniform3f(programId, "simpleLight.position", simpleLightPos);
    renderer.setUniform3f(programId, "simpleLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "simpleLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "simpleLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    renderer.setCameraPos(glm::vec3(0.0f, 0.0f, -10.0f));
}

void Application::prepareMultipleLighting()
{
    addLightingInfo();

    renderer.generateProgram(programId, "Shaders/LightingVS.glsl", "Shaders/MultipleLightingFS.glsl");
    renderer.generateProgram(lightProgramId, "Shaders/LightingVS.glsl", "Shaders/LightingLightFS.glsl");

    std::random_device randomDevice = std::random_device();
    std::default_random_engine randomEngine = std::default_random_engine(randomDevice());

    cubePositions = std::vector<glm::vec3>();
    cubePositions.reserve(cubeCountML);
    std::uniform_real_distribution<float> posDistrib = std::uniform_real_distribution<float>(-10.0f, 10.0f);
    for (unsigned int i = 0; i < cubeCountML; i++)
    {
        float x = posDistrib(randomEngine);
        float y = posDistrib(randomEngine);
        float z = posDistrib(randomEngine);
        cubePositions.push_back(glm::vec3(x, y, z));
    }

    cubeRotationSpeeds = std::vector<glm::vec3>();
    cubeRotationSpeeds.reserve(cubeCountML);
    std::uniform_real_distribution<float> speedDistrib = std::uniform_real_distribution<float>(0.0f, 2.0f);
    for (unsigned int i = 0; i < cubeCountML; i++)
    {
        float x = speedDistrib(randomEngine);
        float y = speedDistrib(randomEngine);
        float z = speedDistrib(randomEngine);
        cubeRotationSpeeds.push_back(glm::vec3(x, y, z));
    }

    directionalLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);

    renderer.setUniform3f(programId, "viewPos", renderer.getCameraPos());

    renderer.setUniform1i(programId, "material.diffuse", 0);
    renderer.setUniform1i(programId, "material.specular", 1);
    renderer.setUniform1f(programId, "material.shininess", 32.0f);

    renderer.setUniform3f(programId, "directionalLight.direction", directionalLightDir);
    renderer.setUniform3f(programId, "directionalLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "directionalLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "directionalLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
}

void Application::prepareForRun()
{
    renderer.prepareForRun();
}

void Application::runGettingStarted()
{
    renderer.prepareForRender();
    renderer.calculateCameraTransform();

    renderer.prepareForDraw(programId, textureIds, vaoId);

    uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();

    
    for (unsigned int i = 0; i < cubeCountGS; i++)
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

void Application::runSimpleLighting()
{
    renderer.prepareForRender();
    renderer.calculateCameraTransform();

    glm::mat4 model;

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);

    model = glm::mat4(1.0f);
    /*model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));*/
    renderer.updateModelMatrix(model);
    renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(model)));
    renderer.applyMvp(programId, "model", "view", "projection");
    renderer.setUniform3f(programId, "viewPos", renderer.getCameraPos());
    renderer.draw(36);

    renderer.unprepareForDraw(programId, textureIds);

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(lightProgramId, textureIds, vaoId);

    model = glm::mat4(1.0f);
    model = glm::translate(model, simpleLightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    renderer.updateModelMatrix(model);
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);

    // -------------------------------------------------------------------------

    renderer.calculateFps();
    renderer.updateGLFW();
}

void Application::runMultipleLighting()
{
    renderer.prepareForRender();
    renderer.calculateCameraTransform();

    glm::mat4 model;

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);
    
    //model = glm::mat4(1.0f);
    ///*model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));*/
    //renderer.updateModelMatrix(model);
    //renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(model)));
    //renderer.applyMvp(programId, "model", "view", "projection");
    //renderer.setUniform3f(programId, "viewPos", renderer.getCameraPos());
    //renderer.draw(36);

    uint64_t milliseconds = renderer.getMillisecondsSinceRunPreparation();
    for (unsigned int i = 0; i < cubeCountML; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions.at(i));

        glm::vec3 rotationSpeed = cubeRotationSpeeds.at(i);
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, milliseconds / 1000.0f * rotationSpeed.z, glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.updateModelMatrix(model);
        renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(model)));
        renderer.applyMvp(programId, "model", "view", "projection");
        renderer.setUniform3f(programId, "viewPos", renderer.getCameraPos());
        renderer.draw(36);
    }

    renderer.unprepareForDraw(programId, textureIds);

    // -------------------------------------------------------------------------

    /*renderer.prepareForDraw(lightProgramId, textureIds, vaoId);

    model = glm::mat4(1.0f);
    model = glm::translate(model, simpleLightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    renderer.updateModelMatrix(model);
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);*/

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