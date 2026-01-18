#include "SimpleLighting.hpp"

SimpleLighting::SimpleLighting()
{

}

void SimpleLighting::prepare()
{
    addLightingInfo();

    renderer.generateProgram(programId, "Shaders/LightingVS.glsl", "Shaders/SimpleLightingFS.glsl");
    renderer.generateProgram(lightProgramId, "Shaders/LightingVS.glsl", "Shaders/LightingLightFS.glsl");

    simpleLightPos = glm::vec3(1.2f, 1.0f, 2.0f);

    Camera camera = camController.getCamera();
    renderer.setUniform3f(programId, "viewPos", camera.pos);

    renderer.setUniform1i(programId, "material.diffuse", 0);
    renderer.setUniform1i(programId, "material.specular", 1);
    renderer.setUniform1f(programId, "material.shininess", 32.0f);

    renderer.setUniform3f(programId, "simpleLight.position", simpleLightPos);
    renderer.setUniform3f(programId, "simpleLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    renderer.setUniform3f(programId, "simpleLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    renderer.setUniform3f(programId, "simpleLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    camera.pos = glm::vec3(0.0f, 0.0f, -10.0f);

	renderer.prepareForRun();
}

void SimpleLighting::addLightingInfo()
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
    renderer.generateResourceTexture2D(texture0, "Resources/TutorialDiffuseMap.png", true, GL_TEXTURE_2D, 0);
    uint32_t texture1;
    renderer.generateResourceTexture2D(texture1, "Resources/TutorialSpecularMap.png", true, GL_TEXTURE_2D, 1);
    textureIds.clear();
    textureIds.push_back(texture0);
    textureIds.push_back(texture1);
}

void SimpleLighting::run()
{
    renderer.prepareForFrame();
    glEnable(GL_DEPTH_TEST);

    glm::mat4 model;
    Camera camera = camController.getCamera();
    glm::mat4 view = camera.getView();

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(programId, textureIds, vaoId);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model = glm::mat4(1.0f);
    /*model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));*/
    renderer.updateModelMatrix(model);
    renderer.setUniformMatrix4fv(programId, "normalMatrix", glm::transpose(glm::inverse(model)));
    renderer.updateViewMatrix(view);
    renderer.applyMvp(programId, "model", "view", "projection");
    renderer.setUniform3f(programId, "viewPos", camera.pos);
    renderer.draw(36);

    renderer.unprepareForDraw(programId, textureIds);

    // -------------------------------------------------------------------------

    renderer.prepareForDraw(lightProgramId, textureIds, vaoId);

    model = glm::mat4(1.0f);
    model = glm::translate(model, simpleLightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    renderer.updateModelMatrix(model);
    renderer.updateViewMatrix(view);
    renderer.applyMvp(lightProgramId, "model", "view", "projection");
    renderer.draw(36);

    renderer.unprepareForDraw(lightProgramId, textureIds);

    // -------------------------------------------------------------------------

    renderer.unprepareForFrame();
    
    window.updateGLFW();
    camController.updateCamera(window.getInputState(), renderer.getFrameTimeMilliseconds());
}

bool SimpleLighting::shouldEnd()
{
    return window.getShouldClose();
}
void SimpleLighting::terminate()
{
    window.terminate();
}