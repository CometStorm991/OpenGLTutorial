#include "Renderer.hpp"

Renderer::Renderer(Camera& camera)
    :
    camera(camera),
    model(glm::mat4(1.0f)),
    view(glm::mat4(1.0f)),
    projection(glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 100.0f)),
    mvp(glm::mat4(1.0f))
{

}

void Renderer::init()
{
    initWindow();
    initOpenGL();
}

GLFWwindow* Renderer::initWindow()
{
    window = nullptr;

    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return window;
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "Failed to create GLFW window" << std::endl;
        return window;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);

    return window;
}

void Renderer::initOpenGL()
{
    if (glewInit() != GLEW_OK) {
        std::cout << "Glew failed." << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Renderer::debugOutputGLFW, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, Renderer::mouseCallbackGLFW);

    glEnable(GL_DEPTH_TEST);
}

void Renderer::generateProgram(uint32_t& programId, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    Program program = Program(vertexShaderPath, fragmentShaderPath);
    program.load();

    programId = program.getId();
    programMap.insert({programId, program});

    program.unuse();
}

void Renderer::generateVertexBuffer(uint32_t& vertexBuffer, const std::vector<float>& vertices)
{
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::generateIndexBuffer(uint32_t& indexBuffer, const std::vector<uint32_t>& indices)
{   
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::generateTexture(uint32_t& textureId, const std::string& imagePath, GLenum pixelFormat)
{
    Texture texture = Texture(imagePath, pixelFormat);
    texture.setup({
        {GL_TEXTURE_WRAP_S, GL_REPEAT},
        {GL_TEXTURE_WRAP_T, GL_REPEAT},
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
        });
    
    textureId = texture.getId();
    textureMap.insert({textureId, texture});
}

// For textures that are used as attachments for framebuffers
void Renderer::generateTexture(uint32_t& textureId, uint32_t width, uint32_t height)
{
    Texture texture = Texture(width, height);
    texture.setup({
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
        });

    textureId = texture.getId();
    textureMap.insert({textureId, texture});
}

void Renderer::generateVertexArray(uint32_t& vaoId, uint32_t vertexBuffer, uint32_t indexBuffer, std::vector<AttributeLayout>& attribs)
{
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    uint32_t vertexSize = 0;
    for (unsigned int i = 0; i < attribs.size(); i++)
    {
        AttributeLayout attrib = attribs.at(i);
        uint32_t typeSize = getGLTypeSize(attrib.getType());

        vertexSize += attrib.getCount() * typeSize;
    }

    unsigned int offset = 0;
    for (unsigned int i = 0; i < attribs.size(); i++)
    {
        AttributeLayout attrib = attribs.at(i);
        uint32_t typeSize = getGLTypeSize(attrib.getType());
        
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, attrib.getCount(), attrib.getType(), GL_FALSE, vertexSize, (const void*)offset);

        offset += attrib.getCount() * typeSize;
    }

    // Binds element buffer in GL_ELEMENT_ARRAY_BUFFER to VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::generateFramebuffer(uint32_t& framebufferId)
{
    glGenFramebuffers(1, &framebufferId);
}

uint32_t Renderer::getGLTypeSize(GLenum type)
{
    switch (type)
    {
    case GL_FLOAT:
        return sizeof(float);
    }

    std::cout << "Could not get size of type " << type << std::endl;
    return 0;
}

void Renderer::prepareForRun()
{   
    startTime = std::chrono::steady_clock::now();
    lastSecondTime = std::chrono::steady_clock::now();
}

void Renderer::updateCameraPosition()
{
    float deltaTime = (milliseconds - previousMillis) / 1000.0f;
    float cameraSpeed = deltaTime * 10.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.pos += cameraSpeed * camera.front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.pos -= cameraSpeed * camera.front;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.pos += cameraSpeed * camera.right;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.pos -= cameraSpeed * camera.right;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.pos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.pos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

void Renderer::prepareForRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    previousMillis = milliseconds;
    milliseconds = getMillisecondsSinceRunPreparation();
}

void Renderer::prepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds, uint32_t vaoId)
{
    programMap.at(programId).use();
    for (unsigned int i = 0; i < textureIds.size(); i++)
    {
        uint32_t textureId = textureIds.at(i);

        textureMap.at(textureId).use(GL_TEXTURE0 + i);
    }
    glBindVertexArray(vaoId);
}

void Renderer::updateModelMatrix(const glm::mat4& model)
{
   this->model = model;
}

void Renderer::applyMvp(uint32_t programId, const std::string& modelName, const std::string& viewName, const std::string& projectionName)
{
    view = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
    setUniformMatrix4fv(programId, modelName, model);
    setUniformMatrix4fv(programId, viewName, view);
    setUniformMatrix4fv(programId, projectionName, projection);
}

void Renderer::draw(unsigned int triangleCount)
{
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::unprepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds)
{
    programMap.at(programId).unuse();
    for (unsigned int i = 0; i < textureIds.size(); i++)
    {
        uint32_t textureId = textureIds.at(i);

        textureMap.at(textureId).unuse(GL_TEXTURE0 + i);
    }
    glBindVertexArray(0);

    //milliseconds = getMillisecondsSinceRunPreparation();
}

void Renderer::calculateFps()
{
    fps++;
    uint32_t currentSecMillis = getMillisecondsSinceTimePoint(lastSecondTime);
    if (currentSecMillis + leftOverMillis > 1000)
    {
        leftOverMillis = currentSecMillis + leftOverMillis - 1000;
        lastSecondTime = std::chrono::steady_clock::now();
        std::cout << "FPS: " << fps << std::endl;
        fps = 0;
    }
}

void Renderer::updateGLFW()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
    updateCameraPosition();
}

void Renderer::terminateGLFW()
{
    glfwTerminate();
}

void Renderer::setUniform1i(uint32_t programId, const std::string& name, int32_t value)
{
    Program program = programMap.at(programId);
    bool wasUsed = program.getBeingUsed();
    program.use();
    glUniform1i(glGetUniformLocation(program.getId(), name.c_str()), value);
    if (!wasUsed)
    {
        program.unuse();
    }
}

void Renderer::setUniform1f(uint32_t programId, const std::string& name, float value)
{
    Program program = programMap.at(programId);
    bool wasUsed = program.getBeingUsed();
    program.use();
    glUniform1f(glGetUniformLocation(program.getId(), name.c_str()), value);
    if (!wasUsed)
    {
        program.unuse();
    }
}

void Renderer::setUniform3f(uint32_t programId, const std::string& name, const glm::vec3& value)
{
    Program program = programMap.at(programId);
    bool wasUsed = program.getBeingUsed();
    program.use();
    glUniform3f(glGetUniformLocation(program.getId(), name.c_str()), value[0], value[1], value[2]);
    if (!wasUsed)
    {
        program.unuse();
    }
}

void Renderer::setUniformMatrix4fv(uint32_t programId, const std::string& name, const glm::mat4& value)
{
    Program program = programMap.at(programId);
    bool wasUsed = program.getBeingUsed();
    program.use();
    glUniformMatrix4fv(glGetUniformLocation(program.getId(), name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    if (!wasUsed)
    {
        program.unuse();
    }
}

void Renderer::testGLM()
{
    glm::vec4 vector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
    vector = trans * vector;
    std::cout << "X: " << vector.x << " Y: " << vector.y << " Z: " << vector.z << std::endl;
}

bool Renderer::getWindowShouldClose()
{
    return glfwWindowShouldClose(window);
}

uint64_t Renderer::getMillisecondsSinceRunPreparation()
{
    return getMillisecondsSinceTimePoint(startTime);
}

void Renderer::mouseCallbackGLFW(GLFWwindow* window, double xPos, double yPos)
{
    Renderer* renderer = (Renderer*)glfwGetWindowUserPointer(window);
    renderer->mouseCallback(window, xPos, yPos);
}

void Renderer::mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos; // Reversed because y coordinates range bottom up
    lastX = xPos;
    lastY = yPos;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    float yaw = camera.getYaw();
    float pitch = camera.getPitch();

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.updateOrientation(yaw, pitch);
}

// This code is from learnopengl.com
void APIENTRY Renderer::debugOutputGLFW(GLenum source,
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