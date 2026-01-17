#include "MultipleLighting.hpp"

MultipleLighting::MultipleLighting()
	: renderer(Renderer(camera))
{
	
}

void MultipleLighting::init()
{
	renderer.init();
}

void MultipleLighting::prepare()
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

    pointLightPos = glm::vec3(1.2f, 1.0f, 2.0f);
    directionalLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);

    renderer.setUniform3f(programId, "viewPos", camera.pos);

    renderer.setUniform1i(programId, "material.diffuse", 0);
    renderer.setUniform1i(programId, "material.specular", 1);
    renderer.setUniform1f(programId, "material.shininess", 32.0f);

    renderer.setUniform3f(programId, "directionalLight.direction", directionalLightDir);
    renderer.setUniform3f(programId, "directionalLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "directionalLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "directionalLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    renderer.setUniform3f(programId, "pointLight.position", pointLightPos);
    renderer.setUniform3f(programId, "pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "pointLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    renderer.setUniform1f(programId, "pointLight.constant", 1.0f);
    renderer.setUniform1f(programId, "pointLight.linear", 0.045f);
    renderer.setUniform1f(programId, "pointLight.quadratic", 0.0075f);

    renderer.setUniform3f(programId, "spotLight.position", camera.pos);
    renderer.setUniform3f(programId, "spotLight.direction", camera.front);
    renderer.setUniform1f(programId, "spotLight.cutoff", glm::cos(glm::radians(12.5f)));
    renderer.setUniform1f(programId, "spotLight.outerCutoff", glm::cos(glm::radians(17.5f)));
    renderer.setUniform3f(programId, "spotLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "spotLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    renderer.setUniform1f(programId, "spotLight.constant", 1.0f);
    renderer.setUniform1f(programId, "spotLight.linear", 0.007f);
    renderer.setUniform1f(programId, "spotLight.quadratic", 0.0002f);
}

void MultipleLighting::addLightingInfo()
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

    renderer.generateVertexArray(vaoId, vertexBuffer, 0, attribs);

    uint32_t texture0;
    renderer.generateResourceTexture(texture0, "Resources/TutorialDiffuseMap.png", true, GL_TEXTURE_2D, 0);
    uint32_t texture1;
    renderer.generateResourceTexture(texture1, "Resources/TutorialSpecularMap.png", true, GL_TEXTURE_2D, 1);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);

    renderer.prepareForRun();
}

void MultipleLighting::run()
{
    renderer.prepareForRender();

    glm::mat4 model;

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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
        camera.updateView();
        renderer.updateViewMatrix(camera.view);
        renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(model)));
        renderer.applyMvp(programId, "model", "view", "projection");
        renderer.setUniform3f(programId, "viewPos", camera.pos);
        renderer.setUniform3f(programId, "spotLight.position", camera.pos);
        renderer.setUniform3f(programId, "spotLight.direction", camera.front);
        renderer.draw(36);
    }

    renderer.unprepareForDraw(programId, textureIds);

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(lightProgramId, textureIds, vaoId);

    model = glm::mat4(1.0f);
    model = glm::translate(model, pointLightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    renderer.updateModelMatrix(model);
    camera.updateView();
    renderer.updateViewMatrix(camera.view);
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);

    // -------------------------------------------------------------------------

    renderer.calculateFps();
    renderer.updateGLFW();
}

bool MultipleLighting::shouldEnd()
{
    return renderer.getWindowShouldClose();
}

void MultipleLighting::terminate()
{
    renderer.terminateGLFW();
}