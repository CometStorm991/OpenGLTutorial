#include "Renderer.hpp"

Renderer::Renderer()
    :
    model(glm::mat4(1.0f)),
    view(glm::mat4(1.0f)),
    projection(glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 100.0f)),
    mvp(glm::mat4(1.0f))
{
    if (glewInit() != GLEW_OK) {
        std::cerr << "[Error]: Glew failed" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Renderer::debugOutputOpenGL, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
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
    glCreateBuffers(1, &vertexBuffer);
    glNamedBufferStorage(vertexBuffer, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
}

void Renderer::generateIndexBuffer(uint32_t& indexBuffer, const std::vector<uint32_t>& indices)
{   
    glCreateBuffers(1, &indexBuffer);
    glNamedBufferStorage(indexBuffer, indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_STORAGE_BIT);
}

void Renderer::generateResourceTexture2D(uint32_t& textureId, const std::string& imagePath, bool flip, GLenum target, uint32_t textureUnit)
{
    Texture texture = Texture::ResourceTexture2D(imagePath, flip, target, textureUnit);
    texture.setup({
        false, {
        {GL_TEXTURE_WRAP_S, GL_REPEAT},
        {GL_TEXTURE_WRAP_T, GL_REPEAT},
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
        } });
    
    textureId = texture.id;
    textureMap.insert({textureId, texture});
}

void Renderer::generateResourceTexture2D(uint32_t& textureId, const std::string& imagePath, bool flip, GLenum internalFormat, GLenum target, uint32_t textureUnit)
{
    Texture texture = Texture::ResourceTexture2D(imagePath, flip, target, textureUnit);
    texture.setup({
        false, internalFormat, {
        {GL_TEXTURE_WRAP_S, GL_REPEAT},
        {GL_TEXTURE_WRAP_T, GL_REPEAT},
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR},
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
        } });

    textureId = texture.id;
    textureMap.insert({ textureId, texture });
}

void Renderer::generateResourceTextureCubemap(uint32_t& textureId, const std::vector<std::string>& imagePaths, bool flip, GLenum target, uint32_t textureUnit)
{
    Texture texture = Texture::ResourceTextureCubemap(imagePaths, flip, target, textureUnit);
    texture.setup({
        true, {
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
        {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
        {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
        {GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE}
        } });

    textureId = texture.id;
    textureMap.insert({ textureId, texture });
}

void Renderer::generateFramebufferTexture(uint32_t& textureId, uint32_t width, uint32_t height)
{
    Texture texture = Texture::FramebufferTexture(width, height);
    texture.setup({
        false, {
        {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
        {GL_TEXTURE_MAG_FILTER, GL_LINEAR}
        } });

    textureId = texture.id;
    textureMap.insert({textureId, texture});
}

void Renderer::generateVertexArray(uint32_t& vaoId, uint32_t vertexBuffer, uint32_t indexBuffer, std::vector<AttributeLayout>& attribs)
{
    glCreateVertexArrays(1, &vaoId);

    uint32_t vertexSize = 0;
    for (unsigned int i = 0; i < attribs.size(); i++)
    {
        AttributeLayout attrib = attribs.at(i);
        uint32_t typeSize = getGLTypeSize(attrib.getType());

        vertexSize += attrib.getCount() * typeSize;
    }

    glVertexArrayVertexBuffer(vaoId, 0, vertexBuffer, 0, vertexSize);
    if (indexBuffer != 0)
    {
        glVertexArrayElementBuffer(vaoId, indexBuffer);
    }

    unsigned int offset = 0;
    for (unsigned int i = 0; i < attribs.size(); i++)
    {
        AttributeLayout attrib = attribs.at(i);
        uint32_t typeSize = getGLTypeSize(attrib.getType());
        
        glEnableVertexArrayAttrib(vaoId, i);
        glVertexArrayAttribFormat(vaoId, i, attrib.getCount(), attrib.getType(), GL_FALSE, offset);
        glVertexArrayAttribBinding(vaoId, i, 0);

        offset += attrib.getCount() * typeSize;
    }
}

void Renderer::generateRenderbuffer(uint32_t& renderbufferId, uint32_t width, uint32_t height)
{
    glCreateRenderbuffers(1, &renderbufferId);
    glNamedRenderbufferStorage(renderbufferId, GL_DEPTH24_STENCIL8, width, height);
}

void Renderer::generateFramebuffer(uint32_t& framebufferId, const std::vector<FbAttachment>& attachments)
{
    glCreateFramebuffers(1, &framebufferId);
    
    for (FbAttachment attachment : attachments)
    {
        switch (attachment.getTargetType())
        {
        case GL_TEXTURE_2D:
            glNamedFramebufferTexture(framebufferId, attachment.getAttachmentPt(), attachment.getAttachmentId(), 0);
            break;
        case GL_RENDERBUFFER:
            glNamedFramebufferRenderbuffer(framebufferId, attachment.getAttachmentPt(), attachment.getTargetType(), attachment.getAttachmentId());
            break;
        }
    }

    if (glCheckNamedFramebufferStatus(framebufferId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "[Error] Framebuffer is not complete!" << std::endl;
    }
}

void Renderer::addTexture(uint32_t& textureId, GLenum target)
{
    Texture texture = Texture::ExternalTexture(textureId, target);
    textureMap.insert({ textureId, texture });
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void Renderer::prepareForFrame()
{
    previousMillis = milliseconds;
    milliseconds = getMillisecondsSinceRunPreparation();
}

void Renderer::prepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds, uint32_t vaoId)
{
    prepareForDraw(0, programId, textureIds, vaoId);
}

void Renderer::prepareForDraw(uint32_t framebufferId, uint32_t programId, const std::vector<uint32_t>& textureIds, uint32_t vaoId)
{
    if (currentFramebuffer != framebufferId)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferId);
        currentFramebuffer = framebufferId;
    }
    
    if (programMap.size() == 0)
    {
        std::cerr << "[Error]: No programs were generated" << std::endl;
    }
    programMap.at(programId).use();

    for (unsigned int i = 0; i < textureIds.size(); i++)
    {
        uint32_t textureId = textureIds.at(i);

        textureMap.at(textureId).use();
    }
    glBindVertexArray(vaoId);
}

void Renderer::updateModelMatrix(const glm::mat4& model)
{
   this->model = model;
}

void Renderer::updateViewMatrix(const glm::mat4& view)
{
    this->view = view;
}

void Renderer::applyMvp(uint32_t programId, const std::string& modelName, const std::string& viewName, const std::string& projectionName)
{
    if (!modelName.empty())
    {
        setUniformMatrix4fv(programId, modelName, model);
    }
    if (!viewName.empty())
    {
        setUniformMatrix4fv(programId, viewName, view);
    }
    if (!projectionName.empty())
    {
        setUniformMatrix4fv(programId, projectionName, projection);
    }
}

void Renderer::draw(unsigned int triangleCount)
{
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::drawInstanced(unsigned int triangleCount, uint32_t instanceCount)
{
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instanceCount);
}

void Renderer::unprepareForDraw(uint32_t programId, const std::vector<uint32_t>& textureIds)
{
    if (currentFramebuffer != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        currentFramebuffer = 0;
    }

    programMap.at(programId).unuse();
    for (unsigned int i = 0; i < textureIds.size(); i++)
    {
        uint32_t textureId = textureIds.at(i);
    }
    glBindVertexArray(0);
}

void Renderer::unprepareForFrame()
{
    fps++;
    uint32_t currentSecMillis = getMillisecondsSinceTimePoint(lastSecondTime);
    if (currentSecMillis + leftOverMillis > 1000)
    {
        leftOverMillis = currentSecMillis + leftOverMillis - 1000;
        lastSecondTime = std::chrono::steady_clock::now();
        std::cout << "FPS: " << fps << " Frame time: " << (1000.0f / fps) << "ms" << std::endl;
        fps = 0;
    }
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

uint32_t Renderer::getFrameTimeMilliseconds()
{
    return milliseconds - previousMillis;
}

uint64_t Renderer::getMillisecondsSinceRunPreparation()
{
    return getMillisecondsSinceTimePoint(startTime);
}

// This code is from learnopengl.com
void APIENTRY Renderer::debugOutputOpenGL(GLenum source,
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