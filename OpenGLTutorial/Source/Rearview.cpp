#include "Rearview.hpp"

Rearview::Rearview()
    : renderer(Renderer(camera))
{

}

void Rearview::init()
{
    renderer.init();
}

void Rearview::prepare()
{
    addLightingInfo();

    renderer.generateProgram(programId, "Shaders/LightingVS.glsl", "Shaders/MultipleLightingFS.glsl");
    renderer.generateProgram(lightProgramId, "Shaders/LightingVS.glsl", "Shaders/LightingLightFS.glsl");

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
    std::uniform_real_distribution<float> speedDistrib = std::uniform_real_distribution<float>(0.0f, 2.0f);
    for (unsigned int i = 0; i < cubeCount; i++)
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

    renderer.prepareForRun();
}

void Rearview::addLightingInfo()
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
    renderer.generateTexture(texture0, "Resources/TutorialDiffuseMap.png", GL_RGBA);
    uint32_t texture1;
    renderer.generateTexture(texture1, "Resources/TutorialSpecularMap.png", GL_RGBA);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    renderer.generateTexture(textureColorBuffer, 1920, 1080);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    uint32_t renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "[Error] Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<float> quadVertices =
    {
        //-1.0f, -1.0f, 0.0f, 0.0f,
        // 1.0f, -1.0f, 1.0f, 0.0f,
        //-1.0f,  1.0f, 0.0f, 1.0f,
        // 1.0f,  1.0f, 1.0f, 1.0f,
         0.6f,  0.6f, 0.0f, 0.0f,
         1.0f,  0.6f, 1.0f, 0.0f,
         0.6f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    std::vector<uint32_t> quadIndices =
    {
        0, 1, 2,
        3, 2, 1,
        //4, 5, 6,
        //7, 6, 5
    };

    uint32_t quadVertexBuffer;
    renderer.generateVertexBuffer(quadVertexBuffer, quadVertices);
    uint32_t quadIndexBuffer;
    renderer.generateIndexBuffer(quadIndexBuffer, quadIndices);
    AttributeLayout quadPosAttrib = AttributeLayout(2, GL_FLOAT);
    AttributeLayout quadTexAttrib = AttributeLayout(2, GL_FLOAT);
    std::vector<AttributeLayout> quadAttribs =
    {
        quadPosAttrib, quadTexAttrib
    };
    
    renderer.generateVertexArray(quadVaoId, quadVertexBuffer, quadIndexBuffer, quadAttribs);

    renderer.generateProgram(quadProgramId, "Shaders/QuadVS.glsl", "Shaders/QuadFS.glsl");
    renderer.setUniform1i(quadProgramId, "screenTexture", 0);
}

void Rearview::run()
{
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    renderer.prepareForRender();

    glm::mat4 model;

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);

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
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);

    // -------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        renderer.prepareForDraw(programId, textureIds, vaoId);

        camera.front = -camera.front;
        camera.right = -camera.right;
        for (unsigned int i = 0; i < cubeCount; i++)
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
        renderer.applyMvp(lightProgramId, "model", "view", "projection");
        renderer.draw(36);

        camera.front = -camera.front;
        camera.right = -camera.right;

        renderer.unprepareForDraw(lightProgramId, textureIds);
    }

    // -------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /*glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);*/

    glUseProgram(quadProgramId);
    glBindVertexArray(quadVaoId);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);

    // -------------------------------------------------------------------------

    renderer.calculateFps();
    renderer.updateGLFW();
}

bool Rearview::shouldEnd()
{
    return renderer.getWindowShouldClose();
}

void Rearview::terminate()
{
    renderer.terminateGLFW();
}