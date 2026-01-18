#include "Window.hpp"

Window::Window()
{
    /* Initialize the library */
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    if (glfwGetCurrentContext() != window)
    {
        glfwTerminate();
        std::cerr << "Failed to make GLFW window the OpenGL context" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, Window::mouseCallbackGLFW);
}

Window::~Window()
{
    glfwTerminate();
}

void Window::updateGLFW()
{
    glfwSwapBuffers(window);
    glfwPollEvents();

    if (!updatedMouse)
    {
        inputState.lastX = inputState.posX;
        inputState.lastY = inputState.posY;
    }
    updatedMouse = false;
    
    recordKeypresses();
}

bool Window::getShouldClose()
{
    return glfwWindowShouldClose(window);
}

void Window::terminate()
{
    glfwTerminate();
}

void Window::recordKeypresses()
{
    inputState.w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    inputState.s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    inputState.d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    inputState.a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    inputState.q = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    inputState.e = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
}

const InputState& Window::getInputState()
{
    return inputState;
}

void Window::mouseCallbackGLFW(GLFWwindow* glfwWindow, double posX, double posY)
{
    Window* window = (Window*)glfwGetWindowUserPointer(glfwWindow);
    window->mouseCallback(posX, posY);
}

void Window::mouseCallback(double posX, double posY)
{
    inputState.lastX = inputState.posX;
    inputState.lastY = inputState.posY;

    inputState.posX = posX;
    inputState.posY = posY;

    updatedMouse = true;
}

