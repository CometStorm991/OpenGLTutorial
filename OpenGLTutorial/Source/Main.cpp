#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numbers>
#include <random>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AttributeLayout.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Renderer.hpp"
#include "Application.hpp"

int main(void)
{
    Application app;
    app.init();
    app.prepare();

    /* Loop until the user closes the window */
    
    while (!app.shouldEnd())
    {
        app.run();
    }
    
    app.terminate();
    return 0;
}